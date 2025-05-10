import cv2
import mediapipe as mp
import os

# New Line == Lift Pen
demo_points = [
    70, 63, 105, 66, 107, 55, 65, 52, 53, # Left Eye Brow
    300, 293, 334, 296, 336, 285, 295, 282, 283, # Right Eye Brow
    414, 286, 258, 257, 259, 260, 467, 359, 381, 380, 374, 373, 390, 249, # Right Eye
    190, 56, 28, 27, 29, 30, 247, 33, 7, 163, 144, 145, 153, 173, # Left Eye
    193, 188, 174, 236, 131, 115, 79, 20, 354, 459, 438, 344, 360, 456, 399, 412, 417, # Nose
    61, 185, 40, 39, 37, 0, 267, 269, 270, 409, 291, 375, 321, 405, 314, 17, 84, 181, 91, 146, # Outline Mouth
    14, 317, 402, 318, 324, 292, 308, 407, 415, 272, 310, 271, 311, 268, 12, 38, 81, 191, 80, 183, 62, 95, 88, 178, 87, # Inside mouth
    10, 338, 297, 332, 284, 251, 389, 356, 454, 323, 361, 288, 397, 365, 379, 378, 400, 377, 152, 148, 176, 149, 150, 136, 172, 58, 132, 93, 234, 127, 162, 21, 54, 103, 67, 109 # Face Outline
]

demo_mode = len(os.sys.argv) > 1 and os.sys.argv[1] == "demo"

# Initialize MediaPipe Face Mesh
mp_face_mesh = mp.solutions.face_mesh
face_mesh = mp_face_mesh.FaceMesh()

# Initialize webcam
cap = cv2.VideoCapture(0)

# Store toggled landmark indices
toggled_points = []

# Store last detected landmarks
last_landmarks = []

# Mouse callback function
def mouse_event(event, x, y, flags, param):
    global toggled_points, last_landmarks
    if event == cv2.EVENT_LBUTTONDOWN and last_landmarks:
        for idx, (lx, ly) in enumerate(last_landmarks):
            if abs(x - lx) < 5 and abs(y - ly) < 5:
                if idx in toggled_points:
                    toggled_points.remove(idx)
                else:
                    toggled_points.append(idx)
                print("Toggled points:", toggled_points)
                break

cv2.namedWindow('Face Landmarks')
cv2.setMouseCallback('Face Landmarks', mouse_event)

while cap.isOpened():
    success, frame = cap.read()
    if not success:
        print("Ignoring empty camera frame.")
        continue

    # Flip the frame horizontally for a mirror effect
    frame = cv2.flip(frame, 1)
    
    # Convert the BGR frame to RGB
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # Process the frame with MediaPipe
    results = face_mesh.process(rgb_frame)

    # Prepare landmark positions for mouse interaction
    last_landmarks = []
    
    if demo_mode:
        # Set the image to all white
        frame.fill(0)
    
    if results.multi_face_landmarks:
        for face_landmarks in results.multi_face_landmarks:
            for idx, landmark in enumerate(face_landmarks.landmark):
                if demo_mode and idx not in demo_points:
                    continue
                
                ih, iw, _ = frame.shape
                x, y = int(landmark.x * iw), int(landmark.y * ih)
                last_landmarks.append((x, y))
                
                # Choose color based on toggle status
                if idx in toggled_points:
                    color = (0, 0, 255)  # Red
                else:
                    color = (0, 255, 0)  # Green
                
                cv2.circle(frame, (x, y), 2, color, -1)

    cv2.imshow('Face Landmarks', frame)

    while cv2.waitKey(1) & 0xFF == 27:
        continue
        

cap.release()
cv2.destroyAllWindows()
