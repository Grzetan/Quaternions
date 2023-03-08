#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <array>

struct Vec3{
    double x, y, z;

    Vec3 operator+(Vec3& v){
        return {x + v.x, y + v.y, z + v.z};
    }

    Vec3 operator-(Vec3& v){
        return {x - v.x, y - v.y, z - v.z};
    }

    Vec3 operator*(double v){
        return {x * v, y * v, z * v};
    }

    Vec3 operator/(double v){
        return {x / v, y / v, z / v};
    }

    double& operator[](unsigned int idx){
        if(idx < 0 || idx > 2) throw std::runtime_error("Index out of range");
        switch(idx){
            case 0:
                return x;
                break;
            case 1:
                return y;
                break;
            case 2:
                return z;
                break;
            default:
                return x;
                break;
        }
    }

    Vec3 crossProduct(Vec3& v){
        return {
            y * v.z - z * v.y,
            x * v.z - z * v.x,
            x * v.y - y * v.x
        };
    }

    double dotProduct(Vec3& v){
        return x * v.x + y * v.y + z * v.z;
    }

    double magnitude(){
        return std::sqrt(x*x + y*y + z*z);
    }

    double distance(Vec3& v){
        return std::sqrt(std::pow(x - v.x, 2) + std::pow(y - v.y, 2) + std::pow(z - v.z, 2));
    }

    friend std::ostream& operator<<(std::ostream& os, const Vec3& dt){
        return os << dt.x << ", " << dt.y << ", " << dt.z << std::endl;
    }
};

struct Quaternion{
    double real, i, j, k;

    Quaternion operator*(Quaternion& q){
        return {
            real*q.real - i*q.i - j*q.j - k*q.k,
            i*q.real + real*q.i + j*q.k - k*q.j,
            real*q.j - i*q.k + j*q.real + k*q.i,
            real*q.k + i*q.j - j*q.i + k*q.real
        };
    }
};

// Frame -> Drone -> X, Y, Z
typedef std::vector<std::vector<Vec3>> MarkerData;

Vec3 computePlaneNormal(std::array<Vec3, 4> points){
    // Find longest arm
    unsigned int longest_a=0, longest_b=1, second_arm = 2;
    double longest_dist = 0;

    for(int i=0; i<4; i++){
        for(int j=i+1; j<4; j++){
            double dist = points[i].distance(points[j]);
            if(dist > longest_dist){
                longest_a = i;
                longest_b = j;
                if(!longest_a == 0 && !longest_b == 0) second_arm = 0;
                else if(!longest_a == 1 && !longest_b == 1) second_arm = 1;
                else if(!longest_a == 2 && !longest_b == 2) second_arm = 2;
                else if(!longest_a == 3 && !longest_b == 3) second_arm = 3;

                longest_dist = dist;
            }
        }
    }

    


}

Quaternion computeRotationQuaternion(Vec3 planeNormal1, Vec3 planeNormal2){
    Vec3 rotationAxis = planeNormal1.crossProduct(planeNormal2);
    double theta = std::acos(planeNormal1.dotProduct(planeNormal2) / (planeNormal1.magnitude() * planeNormal2.magnitude() + 1e-8));

    return {std::cos(theta / 2.0), rotationAxis.x, rotationAxis.y, rotationAxis.z};
}

void parseData(std::string path, MarkerData& data){
    std::ifstream file(path);
    std::string line;

    // Remove header and second line
    std::getline(file, line);
    std::getline(file, line);

    while(std::getline(file, line)){
        std::stringstream ss(line);
        std::string token;
        std::vector<Vec3> frame = {};
        int count = 0;
        Vec3 drone;

        while(std::getline(ss, token, ',')) {
            double val = std::stod(token);

            if(count == 3){
                frame.push_back(drone);
                count = 0;
            }

            drone[count] = val;
            count++;
        }
        frame.push_back(drone); // Add last drone

        data.push_back(frame);
    }
}

int main(){
    // I think desired plane is passed as quarternion so we know it's normal vector
    Vec3 groundPlane = {1,0.56,89};

    MarkerData data;
    parseData("./Dron T02.csv", data);

    const int N_MARKERS = data[0].size();

    for(int i=0; i<5; i++){
        // std::cout << data[i][j][0] << ", " << data[i][j][1] << ", " << data[i][j][2] << std::endl;

        Vec3 planeNormal = computePlaneNormal({data[i][0], data[i][1], data[i][2], data[i][3]});

        // Calculate rotation quarternion between two planes
        Quaternion rotationQuaternion = computeRotationQuaternion(planeNormal, groundPlane);

        std::cout << "Rotation quaternion: r = " 
                << rotationQuaternion.real << ", i = " 
                << rotationQuaternion.i << ", j = " 
                << rotationQuaternion.j << ", k = " 
                << rotationQuaternion.k << std::endl;
    }
}