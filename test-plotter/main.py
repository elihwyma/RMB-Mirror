import cv2
import mediapipe as mp

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

    if results.multi_face_landmarks:
        for face_landmarks in results.multi_face_landmarks:
            for idx, landmark in enumerate(face_landmarks.landmark):
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
