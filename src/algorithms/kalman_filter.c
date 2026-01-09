#include "kalman_filter.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// ==================== 1D KALMAN FILTER ====================
void kalman1d_init(kalman1d_t* kf, float q, float r, float initial_value, float initial_error) {
    if (!kf) return;
    
    kf->q = q;
    kf->r = r;
    kf->x = initial_value;
    kf->p = initial_error;
    kf->k = 0.0f;
    kf->last_update = 0.0f;
    
    printf("[KALMAN1D] Initialized: q=%.6f, r=%.6f, x0=%.3f, p0=%.3f\n",
           q, r, initial_value, initial_error);
}

float kalman1d_update(kalman1d_t* kf, float measurement) {
    if (!kf) return measurement;
    
    // Prediction update
    kf->p = kf->p + kf->q;
    
    // Measurement update
    kf->k = kf->p / (kf->p + kf->r);
    kf->x = kf->x + kf->k * (measurement - kf->x);
    kf->p = (1.0f - kf->k) * kf->p;
    
    return kf->x;
}

void kalman1d_reset(kalman1d_t* kf, float new_value) {
    if (!kf) return;
    
    kf->x = new_value;
    kf->p = 1.0f; // Reset error covariance
    printf("[KALMAN1D] Reset to value: %.3f\n", new_value);
}

float kalman1d_predict(kalman1d_t* kf, float dt) {
    if (!kf) return 0.0f;
    
    // Simple prediction assuming constant value
    // In a real system, you might have a model here
    return kf->x;
}

// ==================== 2D KALMAN FILTER ====================
void kalman2d_init(kalman2d_t* kf, float process_noise, float measurement_noise, 
                   float initial_pos, float initial_vel, float dt) {
    if (!kf) return;
    
    // Initialize state vector
    kf->x[0] = initial_pos;  // Position
    kf->x[1] = initial_vel;  // Velocity
    
    // Initialize covariance matrix
    kf->P[0][0] = 1.0f;
    kf->P[0][1] = 0.0f;
    kf->P[1][0] = 0.0f;
    kf->P[1][1] = 1.0f;
    
    // Process noise covariance
    kf->Q[0][0] = process_noise * dt * dt * dt * dt / 4.0f;
    kf->Q[0][1] = process_noise * dt * dt * dt / 2.0f;
    kf->Q[1][0] = process_noise * dt * dt * dt / 2.0f;
    kf->Q[1][1] = process_noise * dt * dt;
    
    // Measurement noise
    kf->R = measurement_noise;
    
    // State transition matrix (constant velocity model)
    kf->F[0][0] = 1.0f;
    kf->F[0][1] = dt;
    kf->F[1][0] = 0.0f;
    kf->F[1][1] = 1.0f;
    
    // Measurement matrix (we only measure position)
    kf->H[0] = 1.0f;
    kf->H[1] = 0.0f;
    
    kf->last_update = 0.0f;
    
    printf("[KALMAN2D] Initialized: pos0=%.3f, vel0=%.3f, dt=%.3f\n",
           initial_pos, initial_vel, dt);
}

void kalman2d_predict(kalman2d_t* kf, float dt) {
    if (!kf) return;
    
    // Update state transition matrix with new dt
    kf->F[0][1] = dt;
    
    // State prediction: x = F * x
    float new_x0 = kf->F[0][0] * kf->x[0] + kf->F[0][1] * kf->x[1];
    float new_x1 = kf->F[1][0] * kf->x[0] + kf->F[1][1] * kf->x[1];
    
    kf->x[0] = new_x0;
    kf->x[1] = new_x1;
    
    // Covariance prediction: P = F * P * F^T + Q
    float P00 = kf->P[0][0];
    float P01 = kf->P[0][1];
    float P10 = kf->P[1][0];
    float P11 = kf->P[1][1];
    
    // F * P
    float FP00 = kf->F[0][0] * P00 + kf->F[0][1] * P10;
    float FP01 = kf->F[0][0] * P01 + kf->F[0][1] * P11;
    float FP10 = kf->F[1][0] * P00 + kf->F[1][1] * P10;
    float FP11 = kf->F[1][0] * P01 + kf->F[1][1] * P11;
    
    // F * P * F^T
    kf->P[0][0] = FP00 * kf->F[0][0] + FP01 * kf->F[0][1] + kf->Q[0][0];
    kf->P[0][1] = FP00 * kf->F[1][0] + FP01 * kf->F[1][1] + kf->Q[0][1];
    kf->P[1][0] = FP10 * kf->F[0][0] + FP11 * kf->F[0][1] + kf->Q[1][0];
    kf->P[1][1] = FP10 * kf->F[1][0] + FP11 * kf->F[1][1] + kf->Q[1][1];
}

void kalman2d_update(kalman2d_t* kf, float measurement) {
    if (!kf) return;
    
    // Innovation: y = z - H * x
    float y = measurement - (kf->H[0] * kf->x[0] + kf->H[1] * kf->x[1]);
    
    // Innovation covariance: S = H * P * H^T + R
    float S = kf->H[0] * (kf->P[0][0] * kf->H[0] + kf->P[0][1] * kf->H[1]) +
              kf->H[1] * (kf->P[1][0] * kf->H[0] + kf->P[1][1] * kf->H[1]) + kf->R;
    
    // Kalman gain: K = P * H^T * S^-1
    float S_inv = 1.0f / S;
    kf->K[0] = (kf->P[0][0] * kf->H[0] + kf->P[0][1] * kf->H[1]) * S_inv;
    kf->K[1] = (kf->P[1][0] * kf->H[0] + kf->P[1][1] * kf->H[1]) * S_inv;
    
    // State update: x = x + K * y
    kf->x[0] = kf->x[0] + kf->K[0] * y;
    kf->x[1] = kf->x[1] + kf->K[1] * y;
    
    // Covariance update: P = (I - K * H) * P
    float KH00 = kf->K[0] * kf->H[0];
    float KH01 = kf->K[0] * kf->H[1];
    float KH10 = kf->K[1] * kf->H[0];
    float KH11 = kf->K[1] * kf->H[1];
    
    float P00 = kf->P[0][0];
    float P01 = kf->P[0][1];
    float P10 = kf->P[1][0];
    float P11 = kf->P[1][1];
    
    kf->P[0][0] = (1.0f - KH00) * P00 - KH01 * P10;
    kf->P[0][1] = (1.0f - KH00) * P01 - KH01 * P11;
    kf->P[1][0] = -KH10 * P00 + (1.0f - KH11) * P10;
    kf->P[1][1] = -KH10 * P01 + (1.0f - KH11) * P11;
}

float kalman2d_get_position(kalman2d_t* kf) {
    return kf ? kf->x[0] : 0.0f;
}

float kalman2d_get_velocity(kalman2d_t* kf) {
    return kf ? kf->x[1] : 0.0f;
}

// ==================== FILTER HELPER FUNCTIONS ====================
float complementary_filter(float accel, float gyro, float dt, float alpha) {
    // Complementary filter combines accelerometer and gyroscope data
    // alpha determines the weight given to each sensor
    static float angle = 0.0f;
    
    // Integrate gyroscope to get angle change
    float gyro_angle = angle + gyro * dt;
    
    // Use accelerometer for low-frequency correction
    angle = alpha * gyro_angle + (1.0f - alpha) * accel;
    
    return angle;
}

float moving_average(float* buffer, uint32_t size, float new_value) {
    static uint32_t index = 0;
    static uint32_t count = 0;
    static float sum = 0.0f;
    
    if (size == 0) return new_value;
    
    // Update sum: subtract oldest, add newest
    if (count >= size) {
        sum -= buffer[index];
    }
    
    buffer[index] = new_value;
    sum += new_value;
    
    // Update index and count
    index = (index + 1) % size;
    if (count < size) {
        count++;
    }
    
    return sum / count;
}

float low_pass_filter(float input, float prev_output, float alpha) {
    // Simple first-order low-pass filter
    // alpha = dt / (RC + dt)
    return alpha * input + (1.0f - alpha) * prev_output;
}

float high_pass_filter(float input, float prev_input, float prev_output, float alpha) {
    // Simple first-order high-pass filter
    return alpha * (prev_output + input - prev_input);
}

// ==================== 3D KALMAN FILTER (SIMPLIFIED) ====================
void kalman3d_init_simple(kalman3d_t* kf, float pos_noise, float vel_noise, 
                          float measurement_noise, float dt) {
    if (!kf) return;
    
    // Initialize state vector to zeros
    memset(kf->x, 0, sizeof(kf->x));
    
    // Initialize covariance matrix as identity
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            kf->P[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
    
    // Initialize process noise
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            kf->Q[i][j] = 0.0f;
        }
    }
    
    // Set diagonal elements
    for (int i = 0; i < 3; i++) {
        kf->Q[i][i] = pos_noise * dt * dt * dt * dt / 4.0f;
        kf->Q[i+3][i+3] = vel_noise * dt * dt;
        kf->Q[i][i+3] = kf->Q[i+3][i] = pos_noise * dt * dt * dt / 2.0f;
    }
    
    // Initialize measurement noise
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            kf->R[i][j] = (i == j) ? measurement_noise : 0.0f;
        }
    }
    
    printf("[KALMAN3D] Simplified initialization complete\n");
}

// Note: Full 3D Kalman implementation is complex and beyond this example's scope
// This provides the structure for a complete implementation