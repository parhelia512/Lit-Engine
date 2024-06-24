def update():
	if IsKeyDown(KeyboardKey.KEY_W):
		entity.position.x += 5 * time.dt
	if IsKeyDown(KeyboardKey.KEY_S):
		entity.position.x -= 5 * time.dt
	if IsKeyDown(KeyboardKey.KEY_A):
		entity.position.z += 5 * time.dt
	if IsKeyDown(KeyboardKey.KEY_D):
		entity.position.z -= 5 * time.dt




