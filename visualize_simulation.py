import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Load data from multiple files
filenames = ['simulation_data_0.csv', 'simulation_data_1.csv', 'simulation_data_2.csv', 'simulation_data_3.csv']
parsed_data = []

for filename in filenames:
    data = pd.read_csv(filename, header=None, names=["Type", "X", "Y", "Energy"])
    parsed_data.append(data)

# Helper function to parse data by ticks
def parse_data(data):
    ticks = {}
    current_tick = None
    for index, row in data.iterrows():
        if "Tick" in row["Type"]:
            current_tick = int(row["X"])
            ticks[current_tick] = {"wolves": [], "sheep": [], "patches": []}
        else:
            entity = {
                "x": row["X"],
                "y": row["Y"],
                "energy": row["Energy"] if row["Type"] != "Patch" else None,
            }
            if row["Type"] == "Wolf":
                ticks[current_tick]["wolves"].append(entity)
            elif row["Type"] == "Sheep":
                ticks[current_tick]["sheep"].append(entity)
            elif row["Type"] == "Patch":
                ticks[current_tick]["patches"].append({"x": row["X"], "y": row["Y"]})
    return ticks

# Parse the data for all nodes
ticks_data = [parse_data(data) for data in parsed_data]

# Combine parsed data from all nodes
combined_ticks = {}

for tick_data in ticks_data:
    for tick, entities in tick_data.items():
        if tick not in combined_ticks:
            combined_ticks[tick] = {"wolves": [], "sheep": [], "patches": []}
        combined_ticks[tick]["wolves"].extend(entities["wolves"])
        combined_ticks[tick]["sheep"].extend(entities["sheep"])
        combined_ticks[tick]["patches"].extend(entities["patches"])

# Initialize the plot
fig, ax = plt.subplots(figsize=(8, 8))
plt.subplots_adjust(right=0.75)
ax.set_xlim(0, 99)  # ***********Update based on your grid size
ax.set_ylim(0, 99)  # ***********Update based on grid size
ax.set_title("Wolf Sheep Predation Simulation (MPI & OpenMP)")

# Highlight rows 49 and 50
ax.axhspan(49, 50, color='yellow', alpha=1)

wolf_scatter = ax.scatter([], [], c='red', label="Wolves", s=50)
sheep_scatter = ax.scatter([], [], c='blue', label="Sheep", s=30)
patch_scatter = ax.scatter([], [], c='green', label="Grass", s=10)

ax.legend(loc='upper left', bbox_to_anchor=(1.05, 1), title="Entities")

# Add a dynamic text element for counts
text = fig.text(0.80, 0.5, '', fontsize=12, verticalalignment='center', 
                bbox=dict(facecolor='white', alpha=0.8))

# Update function for animation
def update(frame):
    combined_ticks_per_frame = combined_ticks[frame]
    # Wolves
    wolves_x = [wolf["x"] for wolf in combined_ticks_per_frame["wolves"]]
    wolves_y = [wolf["y"] for wolf in combined_ticks_per_frame["wolves"]]
    wolf_scatter.set_offsets(list(zip(wolves_x, wolves_y)))
    
    # Sheep
    sheep_x = [sheep["x"] for sheep in combined_ticks_per_frame["sheep"]]
    sheep_y = [sheep["y"] for sheep in combined_ticks_per_frame["sheep"]]
    sheep_scatter.set_offsets(list(zip(sheep_x, sheep_y)))
    
    # Patches
    patch_x = [patch["x"] for patch in combined_ticks_per_frame["patches"]]
    patch_y = [patch["y"] for patch in combined_ticks_per_frame["patches"]]
    patch_scatter.set_offsets(list(zip(patch_x, patch_y)))
    
    ax.set_title(f"Tick: {frame}")

    # Update text with counts
    num_wolves = len(combined_ticks_per_frame["wolves"])
    num_sheep = len(combined_ticks_per_frame["sheep"])
    num_grass = len(combined_ticks_per_frame["patches"])
    text.set_text(f"Tick: {frame}\nWolves: {num_wolves}\nSheep: {num_sheep}\nGrass: {num_grass}")

# Create animation
anim = FuncAnimation(fig, update, frames=len(combined_ticks), interval=1000)

# Show animation
plt.show()
