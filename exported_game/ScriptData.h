#include "../include_all.h"
std::map<std::string, const char*> scriptMap = {
    {"coins collector0", R"(
import math

velocity = 10.0
dragging_item = None
hovered_entity = None

target_distance = 5.0  # Adjust the distance at which the object appears in front of the entity
gravity = 9.8

yaw = 0.0
pitch = 0.0

grounded = False

rotation_angle = 45.0  # Angle in degrees for rotation

entity.visible = True
def update():
	global velocity, dragging_item, hovered_entity, yaw, pitch, grounded


	camera_direction = camera.front * time.dt * velocity
	camera_direction.y = 0

	DeltaTimeVec3 = Vector3(time.dt, time.dt, time.dt)

	if IsKeyDown(KeyboardKey.KEY_W):
		entity.applyImpulse(camera_direction)

	if IsKeyDown(KeyboardKey.KEY_S):
		entity.applyImpulse(camera.back * DeltaTimeVec3 * velocity)

	if IsKeyDown(KeyboardKey.KEY_A):
		left = camera.left * DeltaTimeVec3 * velocity
		entity.applyImpulse(left)

	if IsKeyDown(KeyboardKey.KEY_D):
		right = camera.right * DeltaTimeVec3 * velocity
		entity.applyImpulse(right)

	if IsKeyPressed(KeyboardKey.KEY_SPACE):
		if (grounded):
			entity.applyImpulse(Vector3(0, 9, 0))  # Adjust for desired jump height

	camera.position = Vector3(entity.position.x + 0, entity.position.y - 0, entity.position.z)
	camera.look_at = Vector3(entity.position.x, entity.position.y, entity.position.z)

	sensitivity = 0.3
	yaw -= GetMouseMovement().x * sensitivity
	pitch -= GetMouseMovement().y * sensitivity

	pitch = max(-89.0, min(89.0, pitch))

	# Calculate the front direction based on yaw and pitch
	front = Vector3(
		math.cos(math.radians(yaw)) * math.cos(math.radians(pitch)),
		math.sin(math.radians(pitch)),
		-math.sin(math.radians(yaw)) * math.cos(math.radians(pitch))
	)

	camera.look_at = camera.position + front
	camera.up = Vector3(0, 1, 0)

	distance = raycast(
						entity.position,
						Vector3(0,-1,0),
						ignore = [entity]
					).distance

	grounded = distance < entity.scale.y / 2 + 0.01

	monkeyRay = raycast(
								entity.position,
								front,
								ignore = [entity]
							)
	
	if (monkeyRay.hit and 
		monkeyRay.distance < .5 and
		monkeyRay.entity.name == "monkey"):
			print(monkeyRay.entity.name)
			monkeyRay.entity.color = Color(0,255,255,255)



















)"},
};
