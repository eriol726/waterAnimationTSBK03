#ifndef CONTROLS_HPP
#define CONTROLS_HPP

class Camera {
public:

	glm::vec3 up;

	// Initial position : on +Z
	glm::vec3 position = glm::vec3(0, 0, 8);
	glm::vec3 direction;

	Camera();
	~Camera();
	void computeMatricesFromInputs(bool invertPitch, float distance);
	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();
	glm::vec3 getCameraPosition();
	void invertPitch();

private:
	glm::mat4 ViewMatrix;
	glm::mat4 ProjectionMatrix;

	
	// Initial horizontal angle : toward -Z
	float horizontalAngle = 3.14f;
	// Initial vertical angle : none
	float verticalAngle = 0.0f;
	// Initial Field of View
	float initialFoV = 45.0f;

	float speed = 3.0f; // 3 units / second
	float mouseSpeed = 0.005f;

	
};


#endif