# Wolf Sheep Predation HPC Project

*Note: This model, wolf-sheep-predation, was sampled from [Netlogo](https://netlogoweb.org/launch#https://netlogoweb.org/assets/modelslib/Sample%20Models/Biology/Wolf%20Sheep%20Predation.nlogo) for the purposes of applying high performance and code optimizing techniques I was taught at university.*

Once you have the code downloaded, you can adjust model parameters in declare_env.cpp and run the simulation with `make` and `./main` in the terminal. Use `mpiexec -n 4 ./main` to run the simulation with 4 nodes. To visualize the code, you will have to adjust the plot size in visualize_simulation.py to match the size in declare_env.cpp before running the python script. You can always `cd` in the `Vector_Only_Model` directory to run the simulation with the base code for performance comparisons.

## Model Introduction
![wolf-sheep-predation](https://github.com/user-attachments/assets/74549b42-080d-465b-95af-7c7a354637bd)
Watch the full visualization on youtube [here](https://www.youtube.com/watch?v=3hkthotYpto)

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

## Base Code
I benchmarked the base code for 2 worlds: 
- 5000 ticks with a 100x100 grid (initially, 100 sheep & 50 wolves)
- 10 ticks with a 30000x30000 grid (initially, 30000 sheep & 15000 wolves)

**Observations:**<br>
![5000ticks_100grid_Non_MPI](https://github.com/user-attachments/assets/d2bd0419-c80e-4bc1-b6d1-18de927f5002)
![10ticks_30000grid_Non_MPI](https://github.com/user-attachments/assets/509479a9-33bf-44c6-9ceb-c45acb266d3c)<br>
as seen above, as the problem size increases, performance degrades even with much less ticks...

## MPI Model
Used MPI to split the simulation world, i.e., the 2D Grid world of patches, across each process and appropriately transferred animals between the processes when they move beyond their respective grid boundaries each tick. I visualized the MPI code in a similar manner to the base code and verified that the animals are moving between nodes/processes appropriately. There are 2 nodes in this [visualization](https://www.youtube.com/watch?v=DfddploUrGA) setup for a 100x100 grid. The yellow rows indicate the rows 49 and 50 (row 0 of the second node). Animals that move between rows 49 and 50 were actually sent between the nodes for inter-node communication with MPI.

I benchamrked the MPI code for the same 2 worlds as the base code: 
- 5000 ticks with a 100x100 grid (initially, 100 sheep & 50 wolves)
- 10 ticks with a 30000x30000 grid (initially, 30000 sheep & 15000 wolves)

## 4X Improvement with 4 Nodes!
The world with 10 ticks, 30000x30000 grid performed ~4 times better than the base code when spltting the workload across 4 nodes: 
![image](https://github.com/user-attachments/assets/24ba1877-2317-45b2-9ac3-c93f2242487b)

But the world with 5000 ticks, 100x100 grid performed 3x worse with 4 nodes:
![image (1)](https://github.com/user-attachments/assets/0641628e-7b13-4ca4-b8af-8353daa9c63e)

The results are in alignment with MPI theory. MPI shines when we have larger grid worlds and less messaging overhead. In smaller worlds, the workload per process is relatively light and the MPI communication overhead dominates since we're sending animals between processes each tick. This is why we see a performance degradation in the smaller world. This is in alignment with Gustaffson's law since the larger workload benefitted from the extra processing power. Since MPI communication costs grow with the amount of data exchanges, I wonder if the performance improvements would be retained in the larger world...

The results are also in alignment with Amdahl's law since the go() function accounts for at least ~90 of the codebase that can be parallelised and we have ~4x performance improvement with 4 nodes.

## Model Visualization for 500 Ticks
![wolf-sheep-predation](https://github.com/user-attachments/assets/74549b42-080d-465b-95af-7c7a354637bd)
Watch the full visualization on youtube [here](https://www.youtube.com/watch?v=3hkthotYpto)

## Model Visualization with MPI
Watch the visualization on youtube [here](https://www.youtube.com/watch?v=DfddploUrGA)
