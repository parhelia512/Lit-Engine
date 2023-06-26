velocity = 0.5
entity.name = str(entity.position.x)

if (IsKeyDown(KeyboardKey.KEY_A)):
    entity.position.z += velocity * time.dt
    print("Left")

if (IsKeyDown(KeyboardKey.KEY_D)):
    entity.position.z -= velocity * time.dt
    print("Right")

if (IsKeyDown(KeyboardKey.KEY_W)):
    entity.position.x -= velocity * time.dt
    print("Forward")

if (IsKeyDown(KeyboardKey.KEY_S)):
    entity.position.x += velocity * time.dt
    print("Backwards")

if (IsKeyDown(KeyboardKey.KEY_E)):
    entity.scale.y += velocity * time.dt

if (IsKeyDown(KeyboardKey.KEY_R)):
    entity.scale.y -= velocity * time.dt

if IsKeyDown(KeyboardKey.KEY_LEFT_SHIFT):
   velocity = 6
else:
	velocity = 0.5

print(time.dt)
camera.position = Vector3(entity.position.x - 10, entity.position.y + 2, entity.position.z)
camera.target = Vector3(entity.position.x, entity.position.y, entity.position.z)









