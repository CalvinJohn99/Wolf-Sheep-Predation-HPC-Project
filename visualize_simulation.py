import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

# Load data
filename = 'simulation_data.csv'  # Update with your CSV file path
data = pd.read_csv(filename, header=None, names=["Type", "X", "Y", "Energy"])

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

# Parse the simulation data
ticks_data = parse_data(data)

# Initialize the plot
fig, ax = plt.subplots(figsize=(8, 8))
plt.subplots_adjust(right=0.75)
ax.set_xlim(0, 100)  # Update based on your grid size
ax.set_ylim(0, 100)
ax.set_title("Wolf Sheep Predation Simulation")

wolf_scatter = ax.scatter([], [], c='red', label="Wolves", s=50)
sheep_scatter = ax.scatter([], [], c='blue', label="Sheep", s=30)
patch_scatter = ax.scatter([], [], c='green', label="Grass", s=10)

ax.legend(loc='upper left', bbox_to_anchor=(1.05, 1), title="Entities")

# Add a dynamic text element for counts
text = fig.text(0.80, 0.5, '', fontsize=12, verticalalignment='center', 
                bbox=dict(facecolor='white', alpha=0.8))

# Update function for animation
def update(frame):
    tick_data = ticks_data[frame]
    # Wolves
    wolves_x = [wolf["x"] for wolf in tick_data["wolves"]]
    wolves_y = [wolf["y"] for wolf in tick_data["wolves"]]
    wolf_scatter.set_offsets(list(zip(wolves_x, wolves_y)))
    
    # Sheep
    sheep_x = [sheep["x"] for sheep in tick_data["sheep"]]
    sheep_y = [sheep["y"] for sheep in tick_data["sheep"]]
    sheep_scatter.set_offsets(list(zip(sheep_x, sheep_y)))
    
    # Patches
    patch_x = [patch["x"] for patch in tick_data["patches"]]
    patch_y = [patch["y"] for patch in tick_data["patches"]]
    patch_scatter.set_offsets(list(zip(patch_x, patch_y)))
    
    ax.set_title(f"Tick: {frame}")

    # Update text with counts
    num_wolves = len(tick_data["wolves"])
    num_sheep = len(tick_data["sheep"])
    num_grass = len(tick_data["patches"])
    text.set_text(f"Tick: {frame}\nWolves: {num_wolves}\nSheep: {num_sheep}\nGrass: {num_grass}")


# Create animation
anim = FuncAnimation(fig, update, frames=len(ticks_data), interval=200)

# Show animation
plt.show()
