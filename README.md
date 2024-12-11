# Wolf Sheep Predation HPC Project

*Note: This model, wolf-sheep-predation, was sampled from [Netlogo](https://netlogoweb.org/launch#https://netlogoweb.org/assets/modelslib/Sample%20Models/Biology/Wolf%20Sheep%20Predation.nlogo) for the purposes of applying high performance and code optimizing techniques I was taught at university.*

## Model Introduction
![wolf-sheep-predation](https://github.com/user-attachments/assets/74549b42-080d-465b-95af-7c7a354637bd)

The aim of the above wolf-sheep-predation model is to simulate stable predator prey dynamics between wolf and sheep.

The system is called unstable if it tends to result in extinction for one or more species involved. In contrast, a system is stable if it tends to maintain itself over time, despite fluctuations in population sizes.

The simulation environment is a 2D grid of patches where each patch object contains a color, green/brown, and a countdown for grass regrowth . A green patch turns brown when eaten and needs to “regrow”. 

We have animals in the simulation, they have energy and a position. They can move around, consume resources, and reproduce. Sheep eat grass while wolves eat sheep. Animals lose energy when they move and die if their energy goes below 0.

The simulation has stopping conditions for ending the simulation: 
- If all wolves and sheep die, the simulation ends.
- If all the wolves die and the number of sheep exceed a certain “max-sheep”, the simulation ends: “The sheep have inherited the earth”.

Important Model Parameters: 
- MODEL-VERSION: Whether sheep, wolves, and grass are modelled (“sheep-wolves-grass”) or only sheep and wolves are modelled (“sheep-wolves”).
- INITIAL-NUMBER-SHEEP, INITIAL-NUMBER-WOLVES: Initial size of sheep and wolf population respectively.
- SHEEP-REPRODUCE, WOLF-REPRODUCE: Amount of energy sheep gain from each grass patch eaten, amount of energy wolves gain for eating one sheep respectively.
- GRASS-REGROWTH-TIME: Time taken for grass to regrow.

## Model Visualization for 500 Ticks
![wolf-sheep-predation](https://github.com/user-attachments/assets/74549b42-080d-465b-95af-7c7a354637bd)
Watch the visualization on youtube [here](https://www.youtube.com/watch?v=3hkthotYpto)

## Model Visualization with MPI
Watch the visualization on youtube [here](https://www.youtube.com/watch?v=DfddploUrGA)
