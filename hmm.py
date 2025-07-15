import random
import matplotlib.pyplot as plt

# HMM parameters
states = ['S1', 'S2', 'S3']  # Three states
observations = ['Red', 'Blue', 'Green']  # Three colors

# Transition probabilities: P(next_state | current_state)
transition_prob = {
    'S1': {'S1': 0.6, 'S2': 0.2, 'S3': 0.2},
    'S2': {'S1': 0.2, 'S2': 0.6, 'S3': 0.2},
    'S3': {'S1': 0.2, 'S2': 0.2, 'S3': 0.6}
}

# Emission probabilities: P(observation | state)
emission_prob = {
    'S1': {'Red': 0.7, 'Blue': 0.2, 'Green': 0.1},
    'S2': {'Red': 0.1, 'Blue': 0.7, 'Green': 0.2},
    'S3': {'Red': 0.2, 'Blue': 0.2, 'Green': 0.6}
}

# Initial state probabilities
start_prob = {'S1': 1/3, 'S2': 1/3, 'S3': 1/3}

def sample_state(prob_dict):
    r = random.random()
    total = 0.0
    for state, prob in prob_dict.items():
        total += prob
        if r < total:
            return state
    return state  # fallback

def simulate_hmm(steps=1000):
    state_sequence = []
    observation_sequence = []

    # Initial state
    current_state = sample_state(start_prob)
    state_sequence.append(current_state)
    current_obs = sample_state(emission_prob[current_state])
    observation_sequence.append(current_obs)

    for _ in range(steps - 1):
        # Transition to next state
        current_state = sample_state(transition_prob[current_state])
        state_sequence.append(current_state)
        # Emit observation
        current_obs = sample_state(emission_prob[current_state])
        observation_sequence.append(current_obs)

    return state_sequence, observation_sequence

def count_frequencies(states_seq, obs_seq):
    # Count occurrences of each color in each state
    freq = {s: {o: 0 for o in observations} for s in states}
    state_counts = {s: 0 for s in states}
    for st, ob in zip(states_seq, obs_seq):
        freq[st][ob] += 1
        state_counts[st] += 1
    # Calculate percentages
    percent = {s: {o: (freq[s][o] / state_counts[s] * 100 if state_counts[s] > 0 else 0) for o in observations} for s in states}
    return percent, freq, state_counts

def print_frequencies(percent):
    print("\nFrequency of each color in each state (%):")
    for s in states:
        print(f"State {s}:")
        for o in observations:
            print(f"  {o}: {percent[s][o]:.1f}%")

def plot_hmm(states_seq, obs_seq):
    steps = len(states_seq)
    x = list(range(steps))
    # Map states and observations to numbers/colors
    state_map = {'S1': 0, 'S2': 1, 'S3': 2}
    obs_map = {'Red': 0, 'Blue': 1, 'Green': 2}
    state_colors = ['red', 'blue', 'green']
    obs_colors = ['pink', 'lightblue', 'lightgreen']

    # Plot states
    plt.figure(figsize=(12, 5))
    plt.subplot(2, 1, 1)
    for s in state_map:
        idx = [i for i, st in enumerate(states_seq) if st == s]
        plt.scatter(idx, [state_map[s]]*len(idx), color=state_colors[state_map[s]], label=f'State {s}', s=10)
    plt.yticks([0, 1, 2], ['S1', 'S2', 'S3'])
    plt.title('State Sequence')
    plt.legend()
    plt.xlabel('Step')
    plt.ylabel('State')
    plt.grid(True, axis='x')

    # Plot observations
    plt.subplot(2, 1, 2)
    for o in obs_map:
        idx = [i for i, ob in enumerate(obs_seq) if ob == o]
        plt.scatter(idx, [obs_map[o]]*len(idx), color=obs_colors[obs_map[o]], label=f'{o}', marker='s', s=10)
    plt.yticks([0, 1, 2], ['Red', 'Blue', 'Green'])
    plt.title('Observation Sequence')
    plt.legend()
    plt.xlabel('Step')
    plt.ylabel('Color')
    plt.grid(True, axis='x')
    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    steps = 1000  # Number of draws for good statistics
    states_seq, obs_seq = simulate_hmm(steps)
    percent, freq, state_counts = count_frequencies(states_seq, obs_seq)
    print_frequencies(percent)
    plot_hmm(states_seq, obs_seq)
