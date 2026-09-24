#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <iomanip>
#include <algorithm>

// ==========================================
// 1. MATEMATIKAI DÍMÉNIÓK ÉS ADATSZERKEZETEK
// ==========================================

// 3D Vektor a játékvilágbeli koordinátákhoz (X, Y, Z)
struct Vector3 {
    float x, y, z;

    Vector3() : x(0), y(0), z(0) {}
    Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    float Distance(const Vector3& other) const {
        return std::sqrt(std::pow(x - other.x, 2) + 
                         std::pow(y - other.y, 2) + 
                         std::pow(z - other.z, 2));
    }
};

// 2D Vektor a képernyő-koordinátákhoz (X, Y)
struct Vector2 {
    float x, y;
    Vector2() : x(0), y(0) {}
    Vector2(float _x, float _y) : x(_x), y(_y) {}
};

// Célzási szögek (Pitch: fel/le, Yaw: balra/jobbra)
struct Vector2Angle {
    float pitch;
    float yaw;
};

// Entitás (Játékos / Ellenség) reprezentációja a memóriában
struct Entity {
    int id;
    std::string name;
    Vector3 position;
    int health;
    bool isVisible;

    Entity(int _id, std::string _name, Vector3 _pos, int _hp, bool _vis)
        : id(_id), name(_name), position(_pos), health(_hp), isVisible(_vis) {}
};

// ==========================================
// 2. MATEMATIKAI ÉS GRAFIKAI ENGINE MOTOR
// ==========================================

class GameSimulationEngine {
private:
    Vector3 localPlayerPos;
    Vector2Angle localPlayerViewAngles;
    std::vector<Entity> entities;
    
    // Képernyő felbontás
    int screenWidth = 1920;
    int screenHeight = 1080;

public:
    GameSimulationEngine() {
        // Lokális játékos kezdőpozíciója a 3D világban
        localPlayerPos = Vector3(0.0f, 0.0f, 0.0f);
        localPlayerViewAngles = { 0.0f, 0.0f };

        // Szimulált ellenségek generálása a 3D térben
        entities.push_back(Entity(1, "Player_Alpha",  Vector3(15.0f, 20.0f, 2.0f),  100, true));
        entities.push_back(Entity(2, "Player_Bravo",  Vector3(-30.0f, 40.0f, -5.0f), 85,  false));
        entities.push_back(Entity(3, "Player_Charlie", Vector3(5.0f, 10.0f, 0.0f),   40,  true));
        entities.push_back(Entity(4, "Player_Delta",   Vector3(100.0f, 120.0f, 10.0f),100, false));
    }

    // --- WORLD TO SCREEN TRANSZFORMÁCIÓ (ESP MŰKÖDÉSE) ---
    // Átalakítja a 3D-s világkoordinátákat 2D-s képernyőpixel-koordinátákká
    bool WorldToScreen(const Vector3& worldPos, Vector2& screenPos) {
        // Egyszerűsített perspektivikus vetítési mátrix modellezése
        float deltaX = worldPos.x - localPlayerPos.x;
        float deltaY = worldPos.y - localPlayerPos.y;
        float deltaZ = worldPos.z - localPlayerPos.z;

        // Ha a célpont a kamera mögött van, nem rajzoljuk ki
        if (deltaY <= 0.1f) return false;

        float fov = 90.0f; // Látószög
        float focalLength = (screenWidth / 2.0f) / std::tan((fov * 0.5f) * (3.14159f / 180.0f));

        // Transzformációs egyenlet
        screenPos.x = (screenWidth / 2.0f) + (deltaX / deltaY) * focalLength;
        screenPos.y = (screenHeight / 2.0f) - (deltaZ / deltaY) * focalLength;

        // Ellenőrzés: a képernyő határain belül van-e
        return (screenPos.x >= 0 && screenPos.x <= screenWidth && 
                screenPos.y >= 0 && screenPos.y <= screenHeight);
    }

    // --- AIMBOT SZÖGKISZÁMÍTÁSI LOGIKA ---
    // Kiszámítja a pontos szöget, amerre a kamerának fordulnia kell
    Vector2Angle CalculateAngle(const Vector3& from, const Vector3& to) {
        Vector2Angle angles;
        Vector3 delta = Vector3(to.x - from.x, to.y - from.y, to.z - from.z);
        
        float hypotenuse = std::sqrt(delta.x * delta.x + delta.y * delta.y);

        // Trigonometriai inverz függvények (Arcus Tangens) használata
        angles.pitch = -std::atan2(delta.z, hypotenuse) * (180.0f / 3.14159f);
        angles.yaw = std::atan2(delta.y, delta.x) * (180.0f / 3.14159f);

        return angles;
    }

    // --- ESP SZIMULÁCIÓS MENTÉS ---
    void RenderESPOverlay() {
        std::cout << "\n=======================================================\n";
        std::cout << "               ESP / OVERLAY SZIMULÁCIÓ                \n";
        std::cout << "=======================================================\n";
        std::cout << std::left << std::setw(12) << "NÉV" 
                  << std::setw(10) << "ÉLET" 
                  << std::setw(12) << "TÁVOLSÁG" 
                  << std::setw(18) << "2D KÉPERNYŐ (X, Y)" 
                  << "LÁTHATÓSÁG\n";
        std::cout << "-------------------------------------------------------\n";

        for (const auto& enemy : entities) {
            Vector2 screenCoords;
            bool isOnScreen = WorldToScreen(enemy.position, screenCoords);
            float dist = localPlayerPos.Distance(enemy.position);

            std::cout << std::left << std::setw(12) << enemy.name 
                      << std::setw(10) << (std::to_string(enemy.health) + " HP")
                      << std::setw(12) << (std::to_string((int)dist) + "m");

            if (isOnScreen) {
                std::string coordsStr = "[" + std::to_string((int)screenCoords.x) + ", " + std::to_string((int)screenCoords.y) + "]";
                std::cout << std::setw(18) << coordsStr;
            } else {
                std::cout << std::setw(18) << "[KÉPERNYŐN KÍVÜL]";
            }

            std::cout << (enemy.isVisible ? "LÁTHATÓ (ZÖLD)" : "REJTETT (PIROS)") << "\n";
        }
    }

    // --- AIMBOT SZIMULÁCIÓ ---
    void ExecuteAimbotSimulation(float fovRadius) {
        std::cout << "\n=======================================================\n";
        std::cout << "              AIMBOT LOCK SZIMULÁCIÓ                   \n";
        std::cout << "=======================================================\n";

        Entity* bestTarget = nullptr;
        float closestDistance = 999999.0f;

        // Célpont kiválasztás: A legközelebbi LÁTHATÓ ellenfél keresése
        for (auto& enemy : entities) {
            if (!enemy.isVisible || enemy.health <= 0) continue;

            float dist = localPlayerPos.Distance(enemy.position);
            if (dist < closestDistance) {
                closestDistance = dist;
                bestTarget = &enemy;
            }
        }

        if (bestTarget != nullptr) {
            Vector2Angle targetAngle = CalculateAngle(localPlayerPos, bestTarget->position);
            
            std::cout << "[+] Célpont befogva: " << bestTarget->name << "\n";
            std::cout << "[+] Célpont Távolsága: " << closestDistance << "m\n";
            std::cout << "[+] Szükséges Kamera Szög (Pitch): " << targetAngle.pitch << " fok\n";
            std::cout << "[+] Szükséges Kamera Szög (Yaw)  : " << targetAngle.yaw << " fok\n";
            
            // Szimulált kamera igazítás
            localPlayerViewAngles = targetAngle;
            std::cout << "[SUCCESS] Célkereszt ráállítva a célpont fejére!\n";
        } else {
            std::cout << "[-] Nincs elérhető/látható célpont a mezőben.\n";
        }
    }
};

// ==========================================
// 3. FŐPROGRAM (MAIN)
// ==========================================

int main() {
    // Console kimenet beállítása
    std::cout << "-------------------------------------------------------\n";
    std::cout << " C++ CHEAT ARCHITEKTÚRA ÉS MATEMATIKAI SZIMULÁTOR      \n";
    std::cout << " (Oktatási & Kiberbiztonsági Szempontú Demonstráció)  \n";
    std::cout << "-------------------------------------------------------\n";

    // Engine inicializálása
    GameSimulationEngine engine;

    // 1. ESP kirajzolási adatok számítása
    engine.RenderESPOverlay();

    // 2. Aimbot célzási logika futtatása (90 fokos FOV kereten belül)
    engine.ExecuteAimbotSimulation(90.0f);

    std::cout << "\n[INFO] A szimuláció sikeresen lefutott.\n";
    return 0;
}
