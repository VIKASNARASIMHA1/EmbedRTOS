#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include <math.h>

// Single dimension Kalman filter
typedef struct {
    float q;      // Process noise covariance
    float r;      // Measurement noise covariance
    float x;      // Estimated value
    float p;      // Estimation error covariance
    float k;      // Kalman gain
    float last_update;
} kalman1d_t;

// 2D Kalman filter (for position/velocity)
typedef struct {
    // State vector [x, vx]
    float x[2];
    
    // Covariance matrix
    float P[2][2];
    
    // Process noise covariance
    float Q[2][2];
    
    // Measurement noise
    float R;
    
    // Kalman gain
    float K[2];
    
    // State transition matrix
    float F[2][2];
    
    // Measurement matrix
    float H[2];
    
    float last_update;
} kalman2d_t;

// 3D Kalman filter (for position/velocity in 3D)
typedef struct {
    // State vector [x, y, z, vx, vy, vz]
    float x[6];
    
    // Covariance matrix 6x6
    float P[6][6];
    
    // Process noise covariance 6x6
    float Q[6][6];
    
    // Measurement noise 3x3
    float R[3][3];
    
    // Kalman gain 6x3
    float K[6][3];
    
    // State transition matrix 6x6
    float F[6][6];
    
    // Measurement matrix 3x6
    float H[3][6];
    
    float last_update;
} kalman3d_t;

// 1D Kalman filter functions
void kalman1d_init(kalman1d_t* kf, float q, float r, float initial_value, float initial_error);
float kalman1d_update(kalman1d_t* kf, float measurement);
void kalman1d_reset(kalman1d_t* kf, float new_value);
float kalman1d_predict(kalman1d_t* kf, float dt);

// 2D Kalman filter functions
void kalman2d_init(kalman2d_t* kf, float process_noise, float measurement_noise, 
                   float initial_pos, float initial_vel, float dt);
void kalman2d_update(kalman2d_t* kf, float measurement);
void kalman2d_predict(kalman2d_t* kf, float dt);
float kalman2d_get_position(kalman2d_t* kf);
float kalman2d_get_velocity(kalman2d_t* kf);

// 3D Kalman filter functions (simplified interface)
void kalman3d_init_simple(kalman3d_t* kf, float pos_noise, float vel_noise, 
                          float measurement_noise, float dt);
void kalman3d_update(kalman3d_t* kf, float x, float y, float z);
void kalman3d_predict(kalman3d_t* kf, float dt);
void kalman3d_get_state(kalman3d_t* kf, float* x, float* y, float* z, 
                        float* vx, float* vy, float* vz);

// Helper functions for sensor fusion
float complementary_filter(float accel, float gyro, float dt, float alpha);
float moving_average(float* buffer, uint32_t size, float new_value);
float low_pass_filter(float input, float prev_output, float alpha);
float high_pass_filter(float input, float prev_input, float prev_output, float alpha);

#endif