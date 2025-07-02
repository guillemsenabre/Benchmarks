import matplotlib.pyplot as plt
import sys

if len(sys.argv) > 1:
    board_name = sys.argv[1]
    values = int(sys.argv[2])
    size = int(sys.argv[3])


# Read N values from file
with open('./files/plot_ns', 'r') as f:
    write_access_times_ns = [int(line.strip()) for line in f if line.strip()]

with open('./files/plot_cycles', 'r') as f:
    write_access_times_cycles = [float(line.strip()) for line in f if line.strip()]

# Sanity check
assert len(write_access_times_ns) == values, f"Expected {values} values in ./files/plot"
assert len(write_access_times_cycles) == values, f"Expected {values} values in ./files/plot"

# Test indices
x = list(range(values))

# Plotting function
def plot(data, name, units):
    plt.figure(figsize=(14, 6))
    plt.plot(x, data, label='Write dataset', marker='s', linestyle='--', linewidth=1)
    plt.title(f'{board_name} - Disk access time per write ({size})')
    plt.xlabel('Test Index')
    plt.ylabel(f'Access Time ({units}/access)')
    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    filename = f'./plots/{name}.png'    # dynamically set filename
    plt.savefig(filename)
    plt.close()  # close the figure to free memory
    print(f"Plot saved to {filename}")

# Making sure './plots/' directory exists or creating it
import os
os.makedirs('./plots', exist_ok=True)

# Call function for each dataset
plot(write_access_times_ns, 'latency_ns', 'ns')
plot(write_access_times_cycles, 'latency_cycles', 'cycles')
