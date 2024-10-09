import argparse
import csv
import sys
from pathlib import Path

import graphviz


class MachineRendererWrapper:
    def __init__(self, comment):
        self.document = graphviz.Digraph(comment=comment)

    def add_state(self, name: str, ext: str):
        self.document.node(name, ext)

    def add_transition(self, src: str, dest: str, label: str):
        self.document.edge(src, dest, label)

    def view(self):
        self.document.view()


class GenericTransition:
    def __init__(self, from_state, to_state, activator):
        self.from_state = from_state
        self.to_state = to_state
        self.signal = activator

    def __hash__(self):
        return hash((self.from_state, self.to_state, self.signal))

    def __eq__(self, other):
        return (self.from_state, self.to_state, self.signal) == (other.from_state, other.to_state, other.signal)

    def __repr__(self):
        return f"T({self.from_state} -> {self.to_state} on {self.signal})"


class MealyMachineStore:
    def __init__(self):
        self.in_signals: [str] = []
        self.states: [str] = []
        self.state_caches: [str] = []
        self.transitions: dict[GenericTransition, str] = dict()

    def generate_renderable(self):
        render = MachineRendererWrapper("Mealy Machine")
        for state in self.states:
            render.add_state(state, state)
        for tr, out in self.transitions.items():
            render.add_transition(tr.from_state, tr.to_state, tr.signal + '/' + out)
        return render

    def write_csv(self, file):
        with open(file, 'w') as f:
            writer = csv.writer(f, delimiter=';')
            ordered_activators = sorted(self.in_signals)
            indexed_activators = dict(zip(ordered_activators, range(len(ordered_activators))))
            ordered_states = sorted(self.states)
            indexed_states = dict(zip(ordered_states, range(len(ordered_states))))
            writer.writerow([''] + ordered_states)
            transitions_matrix = [[activator] + [""] * len(ordered_states) for activator in ordered_activators]
            for tr, out in self.transitions.items():
                transitions_matrix[indexed_activators[tr.signal]] \
                    [indexed_states[tr.from_state.name] + 1] = tr.to_state.name + '/' + out
            writer.writerows(transitions_matrix)


class MooreMachineStore:
    def __init__(self):
        self.in_signals: [str] = []
        self.states: dict[str, str] = dict()
        self.transitions: [GenericTransition] = []

    def generate_renderable(self):
        render = MachineRendererWrapper("Moore Machine")
        for state, out_signal in self.states.items():
            render.add_state(state, state + '/' + out_signal)
        for tr in self.transitions:
            render.add_transition(tr.from_state, tr.to_state, tr.signal)
        return render

    def write_csv(self, file):
        with open(file, 'w') as f:
            writer = csv.writer(f, delimiter=';')
            ordered_activators = sorted(self.in_signals)
            indexed_activators = dict(zip(ordered_activators, range(len(ordered_activators))))
            ordered_states = sorted(self.states.keys())
            indexed_states = dict(zip(ordered_states, range(len(ordered_states))))
            ordered_outputs = [self.states[state] for state in ordered_states]
            writer.writerow([''] + ordered_outputs)
            writer.writerow([''] + ordered_states)
            transitions_matrix = [[activator] + [""] * len(ordered_states) for activator in ordered_activators]
            for tr in self.transitions:
                transitions_matrix[indexed_activators[tr.activator]] \
                    [indexed_states[tr.from_state.name] + 1] = tr.to_state.name
            writer.writerows(transitions_matrix)


def read_mealy_csv(file) -> MealyMachineStore:
    with open(file) as f:
        reader = csv.reader(f, delimiter=';')
        machine = MealyMachineStore()
        machine.states = reader.__next__()[1:]  # ['a1', 'a2', 'a3']
        for line in reader:
            signal = line[0]  # z1
            transitions = line[1:]  # ['a3/w1', 'a1/w1', 'a1/w2']
            machine.in_signals.append(signal)
            for transition, from_state in zip(transitions, machine.states):
                to_state, out_signal = transition.split('/')  # a3, w1
                machine.transitions[GenericTransition(from_state, to_state, signal)] = out_signal
        return machine


def read_moore_csv(file) -> MooreMachineStore:
    with open(file) as f:
        reader = csv.reader(f, delimiter=';')
        machine = MooreMachineStore()
        out_signals = reader.__next__()[1:]
        states = reader.__next__()[1:]
        machine.states = dict(zip(states, out_signals))
        for line in reader:
            signal = line[0]
            to_states = line[1:]
            machine.in_signals.append(signal)
            for from_state, to_state in zip(machine.states, to_states):
                if to_state != '':
                    machine.transitions.append(GenericTransition(from_state, to_state, signal))

        return machine


def render_csv(file):
    with open(file, 'r') as f:
        lines = f.readlines()
    renders_dir = Path(__file__).parent.absolute() / "renders"
    renders_dir.mkdir(parents=True, exist_ok=True)

    if len(lines) > 1 and lines[1].startswith(';'):
        read_moore_csv(file).generate_renderable().document.view(filename=file, directory=renders_dir)
    else:
        read_mealy_csv(file).generate_renderable().document.view(filename=file, directory=renders_dir)


def main():
    parser = argparse.ArgumentParser(description='Render CSV files.')
    parser.add_argument('files', metavar='F', type=str, nargs='+', help='CSV files to render')

    args = parser.parse_args()

    for file in args.files:
        try:
            render_csv(file)
        except Exception as e:
            print(f"An error occurred while processing {file}: {e}", file=sys.stderr)


if __name__ == "__main__":
    main()
