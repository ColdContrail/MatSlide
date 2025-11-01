#define POWITACQ_IMPLEMENTATION
#include <powitacq_rgb.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>

using namespace powitacq_rgb;

const int BRDF_SAMPLING_RES_THETA_H = 90;
const int BRDF_SAMPLING_RES_THETA_D = 90;
const int BRDF_SAMPLING_RES_PHI_D = 360;

const double RED_SCALE = 1.0 / 1500.0;
const double GREEN_SCALE = 1.15 / 1500.0;
const double BLUE_SCALE = 1.66 / 1500.0;

#ifndef M_PI
const double M_PI = 3.1415926535897932384626433832795;
#endif
const int SUBSAMPLE_FACTOR = 2;
void half_diff_coords_to_std_coords(double theta_half, double fi_half, 
                                   double theta_diff, double fi_diff,
                                   double& theta_in, double& fi_in,
                                   double& theta_out, double& fi_out) {

    double h_x = std::sin(theta_half) * std::cos(fi_half);
    double h_y = std::sin(theta_half) * std::sin(fi_half);
    double h_z = std::cos(theta_half);
    
    double d_x = std::sin(theta_diff) * std::cos(fi_diff);
    double d_y = std::sin(theta_diff) * std::sin(fi_diff);
    double d_z = std::cos(theta_diff);
    
    double in_x = d_x * std::cos(theta_half) + d_z * std::sin(theta_half);
    double in_y = d_y;
    double in_z = -d_x * std::sin(theta_half) + d_z * std::cos(theta_half);
    
    double temp_x = in_x * std::cos(fi_half) - in_y * std::sin(fi_half);
    double temp_y = in_x * std::sin(fi_half) + in_y * std::cos(fi_half);
    double temp_z = in_z;
    
    in_x = temp_x;
    in_y = temp_y;
    in_z = temp_z;
    
    theta_in = std::acos(in_z);
    fi_in = std::atan2(in_y, in_x);
    if (fi_in < 0) fi_in += 2 * M_PI;
    
    double dot_product = in_x * h_x + in_y * h_y + in_z * h_z;
    double out_x = 2.0 * dot_product * h_x - in_x;
    double out_y = 2.0 * dot_product * h_y - in_y;
    double out_z = 2.0 * dot_product * h_z - in_z;
    
    theta_out = std::acos(out_z);
    fi_out = std::atan2(out_y, out_x);
    if (fi_out < 0) fi_out += 2 * M_PI;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input.bsdf> <output.binary>" << std::endl;
        return 1;
    }
    
    const char* input_file = argv[1];
    const char* output_file = argv[2];
    
    try {
        BRDF brdf(input_file);
        
        int n = BRDF_SAMPLING_RES_THETA_H * 
                BRDF_SAMPLING_RES_THETA_D * 
                (BRDF_SAMPLING_RES_PHI_D / 2);
        
        std::vector<double> merl_data(3 * n, 0.0);
        
        for (int i = 0; i < BRDF_SAMPLING_RES_THETA_H; ++i) {
            double theta_half = (M_PI / 2.0) * (i * i) / 
                            (BRDF_SAMPLING_RES_THETA_H * BRDF_SAMPLING_RES_THETA_H);
            
            for (int j = 0; j < BRDF_SAMPLING_RES_THETA_D; ++j) {
                // theta_diff 的单元格范围： [j * step_theta, (j+1) * step_theta]
                double step_theta = (M_PI / 2.0) / BRDF_SAMPLING_RES_THETA_D;
                double theta_diff_start = j * step_theta;
                
                for (int k = 0; k < BRDF_SAMPLING_RES_PHI_D / 2; ++k) {
                    // phi_diff 的单元格范围： [k * step_phi, (k+1) * step_phi]
                    double step_phi = M_PI / (BRDF_SAMPLING_RES_PHI_D / 2.0);
                    double phi_diff_start = k * step_phi;
                    
                    // 初始化累加器
                    double red_sum = 0.0, green_sum = 0.0, blue_sum = 0.0;
                    int valid_samples = 0;
                    
                    // 子采样循环
                    for (int sub_j = 0; sub_j < SUBSAMPLE_FACTOR; ++sub_j) {
                        for (int sub_k = 0; sub_k < SUBSAMPLE_FACTOR; ++sub_k) {
                            // 计算子采样点坐标（在单元格内均匀采样）
                            double theta_diff = theta_diff_start + (sub_j + 0.5) * (step_theta / SUBSAMPLE_FACTOR);
                            double phi_diff = phi_diff_start + (sub_k + 0.5) * (step_phi / SUBSAMPLE_FACTOR);
                            
                            double fi_half = 0.0;
                            double theta_in, fi_in, theta_out, fi_out;
                            half_diff_coords_to_std_coords(theta_half, fi_half, theta_diff, phi_diff,
                                                        theta_in, fi_in, theta_out, fi_out);
                            
                            // 检查角度是否有效（在半球内）
                            if (theta_in > M_PI / 2.0 || theta_out > M_PI / 2.0) {
                                continue;
                            }
                            
                            Vector3f wi(
                                std::sin(theta_in) * std::cos(fi_in),
                                std::sin(theta_in) * std::sin(fi_in),
                                std::cos(theta_in)
                            );
                            
                            Vector3f wo(
                                std::sin(theta_out) * std::cos(fi_out),
                                std::sin(theta_out) * std::sin(fi_out),
                                std::cos(theta_out)
                            );
                            
                            Vector3f fr = brdf.eval(wi, wo);
                            
                            red_sum += fr[0];
                            green_sum += fr[1];
                            blue_sum += fr[2];
                            valid_samples++;
                        }
                    }
                    
                    // 计算平均 BRDF 值
                    if (valid_samples > 0) {
                        double red_avg = red_sum / valid_samples;
                        double green_avg = green_sum / valid_samples;
                        double blue_avg = blue_sum / valid_samples;
                        
                        // 应用缩放因子
                        double red = red_avg / RED_SCALE;
                        double green = green_avg / GREEN_SCALE;
                        double blue = blue_avg / BLUE_SCALE;
                        
                        int idx = k + 
                                j * (BRDF_SAMPLING_RES_PHI_D / 2) + 
                                i * (BRDF_SAMPLING_RES_PHI_D / 2) * BRDF_SAMPLING_RES_THETA_D;
                        
                        merl_data[idx] = red;
                        merl_data[idx + n] = green;
                        merl_data[idx + 2 * n] = blue;
                    } else {
                        // 如果没有有效样本，保留默认值（0.0）
                        int idx = k + 
                                j * (BRDF_SAMPLING_RES_PHI_D / 2) + 
                                i * (BRDF_SAMPLING_RES_PHI_D / 2) * BRDF_SAMPLING_RES_THETA_D;
                        merl_data[idx] = 0.0;
                        merl_data[idx + n] = 0.0;
                        merl_data[idx + 2 * n] = 0.0;
                    }
                }
            }
        }
        
        std::ofstream out(output_file, std::ios::binary);
        if (!out) {
            std::cerr << "Cannot open output file: " << output_file << std::endl;
            return 1;
        }
        
        int dims[3] = {
            BRDF_SAMPLING_RES_THETA_H,
            BRDF_SAMPLING_RES_THETA_D,
            BRDF_SAMPLING_RES_PHI_D / 2
        };
        out.write(reinterpret_cast<const char*>(dims), 3 * sizeof(int));
        
        out.write(reinterpret_cast<const char*>(merl_data.data()), 
                 3 * n * sizeof(double));
        
        out.close();
        
        std::cout << "Successfully converted " << input_file 
                  << " to MERL format: " << output_file << std::endl;
                  
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
