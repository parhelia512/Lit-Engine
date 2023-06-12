import collisions_module
import camera_module

entity.name = str(entity.position.x)


if (raycast(collisions_module.Vector3(entity.position.x-2, entity.position.y, entity.position.z), collisions_module.Vector3(1, 0, 0), True)):
    print("you lose")
    entity.visible = False





