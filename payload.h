#ifndef PAYLOAD_H
#define PAYLOAD_H

typedef struct {
    float q_des_abad[4];
    float q_des_hip[4];
    float q_des_knee[4];
    float qd_des_abad[4];
    float qd_des_hip[4];
    float qd_des_knee[4];
    float kp_abad[4];
    float kp_hip[4];
    float kp_knee[4];
    float kd_abad[4];
    float kd_hip[4];
    float kd_knee[4];
    float tau_abad_ff[4];
    float tau_hip_ff[4];
    float tau_knee_ff[4];

    int32_t flag; // [1]
    int32_t count;
    int32_t checksum;
} udp_cmd_t;

typedef struct {
    float q_abad[4];
    float q_hip[4];
    float q_knee[4];
    float qd_abad[4];
    float qd_hip[4];
    float qd_knee[4];//6*4*4=96
    float tau_abad[4];
    float tau_hip[4];
    float tau_knee[4];
    int32_t tmp[12];//108
    int32_t pre[4];//108+16=124
    int32_t flag; // [1]

    float rc_mode;
    float p_des[2]; // (x, y) -1 ~ 1
    float height_variation; // -1 ~ 1
    float v_des[3]; // -1 ~ 1 * (v_scale)
    float rpy_des[3]; // -1 ~ 1
    float omega_des[3]; // -1 ~ 1 * (w_scale)
    float variable[3];
    float step_height; //foot step height 0~2 WYN
    int32_t power;

    int32_t count;
    int32_t checksum;
} udp_data_t;

#endif