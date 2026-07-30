#include "hw06_core.h"

int main() {
    CRTVector cameraDir(0.0f, 0.0f, -1.0f);
    std::cout << "Before pan: " << cameraDir << "\n";

    CRTMatrix3x3 panMatrix = CRTMatrix3x3::rotationY(30.0f);
    CRTVector rotatedDir = panMatrix * cameraDir;

    std::cout << "After 30 degree pan: " << rotatedDir << "\n";
    std::cout << "Length (should stay 1.0): " << rotatedDir.length() << "\n";

    return 0;
}
