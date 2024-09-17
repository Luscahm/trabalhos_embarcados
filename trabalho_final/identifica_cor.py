import asyncio
import numpy as np
import cv2
import io
from PIL import Image
from decouple import config
from viam.robot.client import RobotClient
from viam.rpc.dial import Credentials, DialOptions
from viam.services.vision import VisionClient
from viam.components.camera import Camera
from viam.components.base import Base
from viam.media.utils.pil import pil_to_viam_image, viam_to_pil_image


async def connect():
    opts = RobotClient.Options.with_api_key(
        api_key=config('API_KEY'),
        api_key_id=config('API_KEY_ID')
    )
    return await RobotClient.at_address(config('ADRESS'), opts)

def leftOrRight(detections, midpoint):
    largest_area = 0
    largest = {"x_max": 0, "x_min": 0, "y_max": 0, "y_min": 0}
    if not detections:
        print("nothing detected :(")
        return -1
    for d in detections:
        a = (d.x_max - d.x_min) * (d.y_max-d.y_min)
        if a > largest_area:
            a = largest_area
            largest = d
    centerX = largest.x_min + largest.x_max/2
    if centerX < midpoint-midpoint/6:
        return 0  
    if centerX > midpoint+midpoint/6:
        return 2  
    else:
        return 1  
async def main():
    spinNum = 10        
    straightNum = 300    
    # Só para não rodar a eternamente
    numCycles = 200      
    vel = 500            

    machine = await connect()
    base = Base.from_robot(machine, "viam_base")
    camera_name = "cam"
    camera = Camera.from_robot(machine, camera_name)
    frame = await camera.get_image(mime_type="image/jpeg")

    pil_frame = viam_to_pil_image(frame)

    my_detector = VisionClient.from_robot(machine, "vision-2")

    for i in range(numCycles):
        img = await camera.get_image()

        # Get image data as bytes
        img_bytes = img.data
        pil_img = Image.open(io.BytesIO(img_bytes))
        pix = np.array(pil_img)

        # Convert color format from RGB to BGR for OpenCV
        pix = cv2.cvtColor(pix, cv2.COLOR_RGB2BGR)

        # Perform object detection
        detections = await my_detector.get_detections(img)

        # Put boxes around objects with enough confidence (> 0.6)
        conf = 0.6
        for d in detections:
            if d.confidence > conf:
                cv2.rectangle(pix, (d.x_min, d.y_min), (d.x_max, d.y_max), (0, 0, 255), 3)

        cv2.imshow('object_detect', pix)
        cv2.waitKey(1)

        detections = await my_detector.get_detections_from_camera(camera_name)

        answer = leftOrRight(detections, pil_frame.size[0]/2)
        if answer == 0:
            print("left")
            await base.spin(spinNum, vel)     
            await base.move_straight(straightNum, vel)
        if answer == 1:
            print("center")
            await base.move_straight(straightNum, vel)
        if answer == 2:
            print("right")
            await base.spin(-spinNum, vel)
    await robot.close()

if __name__ == "__main__":
    print("Starting up... ")
    asyncio.run(main())
    print("Done.")