# 3D_openGL_game_SeaWolf
Introduction

The game I chose is called Seawolf, it’s a pixel style game. As figure 1 shows, excluding the static background, the moving objects include ships (player and enemies), fishes , and bullets. While the bullets from the player attacked the enemy ship, the enemy ship would be destroyed and sank, player would get score. 

In my game, as figure 2 shows, we have 2 enemy ships. The green ship is slower but farther, and the pink ship is closer but quicker. The purple ship is the player, we could control the player ship including moving left and right, and shooting bullets. If the enemy ships were attacked by the bullets, the enemy ships would disappear and a new ship would generate from the side of the screen. There is an orange fish between the player and enemy ships, it also moves left and right, if the bullets attacked the fish, the bullets would disappear and couldn’t attack the enemy ships, same as the original game. When the bullets successfully attacked one enemy ship, the player would get one score, upper the screen would add a new red heart, in the figure 2, the player got 5 scores.


3D objects

There are 4 3D objects in my game imported by Assimp. The ship is from github (link: https://github.com/AbhijnanVegi/Shipwreck/tree/e7cf080172a9dd93b2f0576f51b16e11f886aef4 ). The heart, fish, and bullet are from this free 3D object website (link: https://free3d.com).


Transformation

In my game, the moving of objects (including fish, ships, and bullets) are based on the matrix vector transformation in OpenGL. Here, we have 4 matrix parameters to determine the object vertex's position in the game world. And the matrix parameter “Transformation” determines the object scale and position. In this game, we don’t need to change the scale of objects, so we just need to change a number in “Transformation”, in each frame, the “pos_ship1” would add or minus a number “Delta_ship1”, then this object could move left and right in my game world. “Delta_ship1” determines the move speed of the object.


Lighting

The lighting in my game world was calculated in vertex shader, the lighting position and intensity were set up in vertex shader. And based on the object color and position, the lighting color was calculated. Here the lighting position is a little below the viewer’s angle, you can see the near side of the fish is brighter, and the other side is darker.


Change view

As described in the transformation part, we have 4 matrix parameters to determine the object vertex's position in the game world. And the “view” matrix determines the camera position. In this game, when we click the set up key, the “camera_pos_y” and “camera_pos_z” values would change, and we can move the view closer or farther and left or right.


Keyboard control

As described in the lighting part, if we press the “j” and “l” keys,  “camera_pos_z” values would change, then the view would move left or right. If we press the “i” and “k”, “camera_pos_y” values would change, then the view would move closer or farther.

As the transformation part described, if we need to use a keyboard to move the player ship, when displaying each frame, the “pos_ship1” value would not change. But when we press the set up key “a” or “d”, the “pos_ship1” would add or minus the “Delta_ship1”, and the player ship could move left and right. 
If we press the key “t”, we can shoot a bullet from the player ship. When we press the key, “has_bullet” would be set as “true”, and when displaying the frames, we would know now we need to shoot a bullet, the initial position of the bullet is the player ship’s position, after shooting, the bullet would move far from the screen.


Collision Detection 

After shooting, we need to detect whether our bullet attacked the fish and enemy ships. It’s very easy in this game, because the objects just move in one direction. And in every frame, the position of every object was calculated and recorded. We just need to estimate whether their positions overlapped or not. If the answer is true, we know the collision happened. Here, if the bullets attacked the fish, the bullets would disappear. And if the bullet successfully attacked an enemy ship, the bullets would also disappear and the player would get one score.

This project is based on OpenGL version 2.1, running on Xcode macOS Monterey.

demo_link: https://www.youtube.com/watch?v=r16RHoeqjFc&t=151s
