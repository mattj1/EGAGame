
Mouse capture to disable DOSBox

Mouse event system

Alternate sprite render for Raylib version
 - White flash
 - Increase brightness
 - Red flash?


- Spawn two units and have them move instead of just one

- Pathfinding

- Debug drawing
  - Show destination points or entities
  - Show current action/logic

Movement logic
 - If targeting a point, the unit should try to move there - even if being attacked (for now)
 - If the unit is standing/waiting at a point - ???

How should the tile pathfinding work?
 - maybe they brute-force their way onto a tile until they're touching it, and brute-force their way to the next

 - Start by following the tile path
 - If we touch the final tile, we go into line move
 - If we hit the target entity at any point along the move, we can stop
 - 
