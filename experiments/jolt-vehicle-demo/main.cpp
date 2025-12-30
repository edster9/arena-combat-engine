/**
 * Jolt Physics Vehicle Demo
 *
 * Standalone demo to test Jolt's WheeledVehicleController
 * before integrating into tabletop engine.
 */

#include <iostream>
#include <cmath>
#include <thread>
#include <algorithm>

// SDL2 and OpenGL
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <GL/glu.h>

// Jolt Physics
#include <Jolt/Jolt.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Vehicle/WheeledVehicleController.h>
#include <Jolt/Physics/Vehicle/VehicleCollisionTester.h>

// Jolt uses its own namespace
using namespace JPH;
using namespace JPH::literals;

// Layer definitions for collision filtering
namespace Layers
{
    static constexpr ObjectLayer NON_MOVING = 0;
    static constexpr ObjectLayer MOVING = 1;
    static constexpr ObjectLayer NUM_LAYERS = 2;
};

namespace BroadPhaseLayers
{
    static constexpr BroadPhaseLayer NON_MOVING(0);
    static constexpr BroadPhaseLayer MOVING(1);
    static constexpr uint NUM_LAYERS(2);
};

// BroadPhaseLayerInterface implementation
class BPLayerInterfaceImpl final : public BroadPhaseLayerInterface
{
public:
    BPLayerInterfaceImpl()
    {
        mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
        mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
    }

    virtual uint GetNumBroadPhaseLayers() const override { return BroadPhaseLayers::NUM_LAYERS; }

    virtual BroadPhaseLayer GetBroadPhaseLayer(ObjectLayer inLayer) const override
    {
        return mObjectToBroadPhase[inLayer];
    }

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
    virtual const char* GetBroadPhaseLayerName(BroadPhaseLayer inLayer) const override
    {
        switch ((BroadPhaseLayer::Type)inLayer)
        {
        case (BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING: return "NON_MOVING";
        case (BroadPhaseLayer::Type)BroadPhaseLayers::MOVING: return "MOVING";
        default: return "INVALID";
        }
    }
#endif

private:
    BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
};

// ObjectVsBroadPhaseLayerFilter implementation
class ObjectVsBroadPhaseLayerFilterImpl : public ObjectVsBroadPhaseLayerFilter
{
public:
    virtual bool ShouldCollide(ObjectLayer inLayer1, BroadPhaseLayer inLayer2) const override
    {
        switch (inLayer1)
        {
        case Layers::NON_MOVING:
            return inLayer2 == BroadPhaseLayers::MOVING;
        case Layers::MOVING:
            return true;
        default:
            return false;
        }
    }
};

// ObjectLayerPairFilter implementation
class ObjectLayerPairFilterImpl : public ObjectLayerPairFilter
{
public:
    virtual bool ShouldCollide(ObjectLayer inObject1, ObjectLayer inObject2) const override
    {
        switch (inObject1)
        {
        case Layers::NON_MOVING:
            return inObject2 == Layers::MOVING;
        case Layers::MOVING:
            return true;
        default:
            return false;
        }
    }
};

// Contact listener (optional, for debugging)
class MyContactListener : public ContactListener
{
public:
    virtual ValidateResult OnContactValidate(const Body& inBody1, const Body& inBody2,
        RVec3Arg inBaseOffset, const CollideShapeResult& inCollisionResult) override
    {
        return ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void OnContactAdded(const Body& inBody1, const Body& inBody2,
        const ContactManifold& inManifold, ContactSettings& ioSettings) override
    {
    }
};

// Body activation listener (optional)
class MyBodyActivationListener : public BodyActivationListener
{
public:
    virtual void OnBodyActivated(const BodyID& inBodyID, uint64 inBodyUserData) override {}
    virtual void OnBodyDeactivated(const BodyID& inBodyID, uint64 inBodyUserData) override {}
};

// Global physics objects
static TempAllocatorImpl* gTempAllocator = nullptr;
static JobSystemThreadPool* gJobSystem = nullptr;
static PhysicsSystem* gPhysicsSystem = nullptr;
static BPLayerInterfaceImpl* gBroadPhaseLayerInterface = nullptr;
static ObjectVsBroadPhaseLayerFilterImpl* gObjectVsBroadPhaseLayerFilter = nullptr;
static ObjectLayerPairFilterImpl* gObjectLayerPairFilter = nullptr;
static MyContactListener* gContactListener = nullptr;
static MyBodyActivationListener* gBodyActivationListener = nullptr;

// Vehicle
static VehicleConstraint* gVehicleConstraint = nullptr;
static BodyID gVehicleBodyID;

// Camera
static float gCameraDistance = 15.0f;
static float gCameraHeight = 6.0f;
static float gCameraAngle = 0.0f;

// Input state
static bool gKeyForward = false;
static bool gKeyBackward = false;
static bool gKeyLeft = false;
static bool gKeyRight = false;
static bool gKeyBrake = false;

void TraceImpl(const char* inFMT, ...)
{
    va_list list;
    va_start(list, inFMT);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), inFMT, list);
    va_end(list);
    std::cout << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS
bool AssertFailedImpl(const char* inExpression, const char* inMessage, const char* inFile, uint inLine)
{
    std::cout << inFile << ":" << inLine << ": (" << inExpression << ") "
              << (inMessage ? inMessage : "") << std::endl;
    return true; // Break into debugger
}
#endif

void InitPhysics()
{
    // Register allocation hook
    RegisterDefaultAllocator();

    // Install trace and assert callbacks
    Trace = TraceImpl;
    JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

    // Create factory
    Factory::sInstance = new Factory();

    // Register all Jolt physics types
    RegisterTypes();

    // Allocators
    gTempAllocator = new TempAllocatorImpl(10 * 1024 * 1024);
    gJobSystem = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers,
        std::thread::hardware_concurrency() - 1);

    // Layer interfaces
    gBroadPhaseLayerInterface = new BPLayerInterfaceImpl();
    gObjectVsBroadPhaseLayerFilter = new ObjectVsBroadPhaseLayerFilterImpl();
    gObjectLayerPairFilter = new ObjectLayerPairFilterImpl();

    // Create physics system
    const uint cMaxBodies = 1024;
    const uint cNumBodyMutexes = 0;
    const uint cMaxBodyPairs = 1024;
    const uint cMaxContactConstraints = 1024;

    gPhysicsSystem = new PhysicsSystem();
    gPhysicsSystem->Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints,
        *gBroadPhaseLayerInterface, *gObjectVsBroadPhaseLayerFilter, *gObjectLayerPairFilter);

    // Contact and activation listeners
    gContactListener = new MyContactListener();
    gBodyActivationListener = new MyBodyActivationListener();
    gPhysicsSystem->SetContactListener(gContactListener);
    gPhysicsSystem->SetBodyActivationListener(gBodyActivationListener);

    // Set gravity
    gPhysicsSystem->SetGravity(Vec3(0, -9.81f, 0));
}

void CreateGround()
{
    BodyInterface& bodyInterface = gPhysicsSystem->GetBodyInterface();

    // Create a large ground box
    BoxShapeSettings groundShapeSettings(Vec3(100.0f, 1.0f, 100.0f));
    groundShapeSettings.SetEmbedded();
    ShapeSettings::ShapeResult groundShapeResult = groundShapeSettings.Create();
    ShapeRefC groundShape = groundShapeResult.Get();

    BodyCreationSettings groundSettings(groundShape, RVec3(0, -1, 0), Quat::sIdentity(),
        EMotionType::Static, Layers::NON_MOVING);

    Body* ground = bodyInterface.CreateBody(groundSettings);
    bodyInterface.AddBody(ground->GetID(), EActivation::DontActivate);
}

void CreateVehicle()
{
    BodyInterface& bodyInterface = gPhysicsSystem->GetBodyInterface();

    // Vehicle dimensions
    const float halfVehicleLength = 2.0f;
    const float halfVehicleWidth = 0.9f;
    const float halfVehicleHeight = 0.5f;
    const float wheelRadius = 0.35f;
    const float wheelWidth = 0.2f;
    const float suspensionMinLength = 0.1f;
    const float suspensionMaxLength = 0.4f;
    const float suspensionFrequency = 1.5f;
    const float suspensionDamping = 0.5f;

    // Create car body shape with lowered center of mass for stability
    RefConst<Shape> carShape = OffsetCenterOfMassShapeSettings(
        Vec3(0, -0.3f, 0),
        new BoxShape(Vec3(halfVehicleWidth, halfVehicleHeight, halfVehicleLength))
    ).Create().Get();

    // Create the car body
    BodyCreationSettings carBodySettings(carShape, RVec3(0, 2, 0), Quat::sIdentity(),
        EMotionType::Dynamic, Layers::MOVING);
    carBodySettings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
    carBodySettings.mMassPropertiesOverride.mMass = 1500.0f;

    Body* carBody = bodyInterface.CreateBody(carBodySettings);
    gVehicleBodyID = carBody->GetID();
    bodyInterface.AddBody(gVehicleBodyID, EActivation::Activate);

    // Create vehicle constraint settings
    VehicleConstraintSettings vehicleSettings;
    vehicleSettings.mUp = Vec3::sAxisY();
    vehicleSettings.mForward = Vec3::sAxisZ();

    // Wheel positions relative to car center
    const Vec3 wheelPositions[4] = {
        Vec3(-halfVehicleWidth, -0.2f, halfVehicleLength - 0.3f),   // Front Left
        Vec3(halfVehicleWidth, -0.2f, halfVehicleLength - 0.3f),    // Front Right
        Vec3(-halfVehicleWidth, -0.2f, -halfVehicleLength + 0.3f),  // Rear Left
        Vec3(halfVehicleWidth, -0.2f, -halfVehicleLength + 0.3f),   // Rear Right
    };

    // Create wheel settings
    WheelSettingsWV* wheelSettings[4];
    for (int i = 0; i < 4; i++)
    {
        wheelSettings[i] = new WheelSettingsWV();
        wheelSettings[i]->mPosition = wheelPositions[i];
        wheelSettings[i]->mSuspensionDirection = Vec3(0, -1, 0);
        wheelSettings[i]->mSteeringAxis = Vec3(0, 1, 0);
        wheelSettings[i]->mWheelUp = Vec3(0, 1, 0);
        wheelSettings[i]->mWheelForward = Vec3(0, 0, 1);
        wheelSettings[i]->mSuspensionMinLength = suspensionMinLength;
        wheelSettings[i]->mSuspensionMaxLength = suspensionMaxLength;
        wheelSettings[i]->mSuspensionSpring.mFrequency = suspensionFrequency;
        wheelSettings[i]->mSuspensionSpring.mDamping = suspensionDamping;
        wheelSettings[i]->mRadius = wheelRadius;
        wheelSettings[i]->mWidth = wheelWidth;
        wheelSettings[i]->mMaxSteerAngle = (i < 2) ? DegreesToRadians(30.0f) : 0.0f; // Front wheels steer
        wheelSettings[i]->mMaxHandBrakeTorque = (i >= 2) ? 4000.0f : 0.0f; // Rear wheels brake

        vehicleSettings.mWheels.push_back(wheelSettings[i]);
    }

    // Create wheeled vehicle controller settings
    WheeledVehicleControllerSettings* controllerSettings = new WheeledVehicleControllerSettings();

    // Engine settings
    controllerSettings->mEngine.mMaxTorque = 500.0f;
    controllerSettings->mEngine.mMinRPM = 1000.0f;
    controllerSettings->mEngine.mMaxRPM = 6000.0f;

    // Transmission settings (simple automatic)
    controllerSettings->mTransmission.mMode = ETransmissionMode::Auto;
    controllerSettings->mTransmission.mGearRatios = { 2.66f, 1.78f, 1.3f, 1.0f, 0.74f };
    controllerSettings->mTransmission.mReverseGearRatios = { -2.9f };
    controllerSettings->mTransmission.mClutchStrength = 10.0f;

    // Differentials - RWD setup
    controllerSettings->mDifferentials.resize(1);
    controllerSettings->mDifferentials[0].mLeftWheel = 2;  // Rear left
    controllerSettings->mDifferentials[0].mRightWheel = 3; // Rear right
    controllerSettings->mDifferentials[0].mDifferentialRatio = 3.42f;

    vehicleSettings.mController = controllerSettings;

    // Create the vehicle constraint
    gVehicleConstraint = new VehicleConstraint(*carBody, vehicleSettings);

    // Create collision tester for wheels (cylinder cast)
    // Use MOVING layer - this is the layer the cast belongs to, not what it tests against
    // MOVING objects can collide with everything (including NON_MOVING ground)
    RefConst<VehicleCollisionTester> collisionTester = new VehicleCollisionTesterCastCylinder(
        Layers::MOVING, 0.05f);
    gVehicleConstraint->SetVehicleCollisionTester(collisionTester);

    // Add constraint to physics system
    gPhysicsSystem->AddConstraint(gVehicleConstraint);
    gPhysicsSystem->AddStepListener(gVehicleConstraint);
}

void UpdateVehicleInput()
{
    if (!gVehicleConstraint) return;

    WheeledVehicleController* controller = static_cast<WheeledVehicleController*>(
        gVehicleConstraint->GetController());

    // Forward/backward
    float forward = 0.0f;
    if (gKeyForward) forward = 1.0f;
    else if (gKeyBackward) forward = -1.0f;

    // Steering
    float steer = 0.0f;
    if (gKeyLeft) steer = -1.0f;
    else if (gKeyRight) steer = 1.0f;

    // Apply inputs
    controller->SetDriverInput(forward, steer, gKeyBrake ? 1.0f : 0.0f, 0.0f);

    // Keep vehicle awake when driving
    if (forward != 0 || steer != 0)
    {
        BodyInterface& bodyInterface = gPhysicsSystem->GetBodyInterface();
        bodyInterface.ActivateBody(gVehicleBodyID);
    }

    // Debug output
    static int debugFrame = 0;
    if (++debugFrame % 60 == 0)
    {
        BodyInterface& bodyInterface = gPhysicsSystem->GetBodyInterface();
        Vec3 vel = bodyInterface.GetLinearVelocity(gVehicleBodyID);
        float speed = vel.Length();
        bool isActive = bodyInterface.IsActive(gVehicleBodyID);

        // Check wheel contact
        const Wheels& wheels = gVehicleConstraint->GetWheels();
        int contactCount = 0;
        for (uint i = 0; i < wheels.size(); i++)
        {
            if (wheels[i]->HasContact()) contactCount++;
        }

        std::cout << "fwd=" << forward << " steer=" << steer
                  << " spd=" << speed << " rpm=" << controller->GetEngine().GetCurrentRPM()
                  << " gear=" << controller->GetTransmission().GetCurrentGear()
                  << " active=" << isActive << " contacts=" << contactCount << std::endl;
    }
}

void StepPhysics(float deltaTime)
{
    const int cCollisionSteps = 1;
    gPhysicsSystem->Update(deltaTime, cCollisionSteps, gTempAllocator, gJobSystem);
}

void DrawBox(Vec3 halfExtents, RMat44 transform)
{
    Vec3 corners[8] = {
        Vec3(-halfExtents.GetX(), -halfExtents.GetY(), -halfExtents.GetZ()),
        Vec3(halfExtents.GetX(), -halfExtents.GetY(), -halfExtents.GetZ()),
        Vec3(halfExtents.GetX(), halfExtents.GetY(), -halfExtents.GetZ()),
        Vec3(-halfExtents.GetX(), halfExtents.GetY(), -halfExtents.GetZ()),
        Vec3(-halfExtents.GetX(), -halfExtents.GetY(), halfExtents.GetZ()),
        Vec3(halfExtents.GetX(), -halfExtents.GetY(), halfExtents.GetZ()),
        Vec3(halfExtents.GetX(), halfExtents.GetY(), halfExtents.GetZ()),
        Vec3(-halfExtents.GetX(), halfExtents.GetY(), halfExtents.GetZ()),
    };

    // Transform corners
    for (int i = 0; i < 8; i++)
    {
        corners[i] = transform * corners[i];
    }

    glBegin(GL_LINES);
    // Bottom
    glVertex3f(corners[0].GetX(), corners[0].GetY(), corners[0].GetZ());
    glVertex3f(corners[1].GetX(), corners[1].GetY(), corners[1].GetZ());
    glVertex3f(corners[1].GetX(), corners[1].GetY(), corners[1].GetZ());
    glVertex3f(corners[2].GetX(), corners[2].GetY(), corners[2].GetZ());
    glVertex3f(corners[2].GetX(), corners[2].GetY(), corners[2].GetZ());
    glVertex3f(corners[3].GetX(), corners[3].GetY(), corners[3].GetZ());
    glVertex3f(corners[3].GetX(), corners[3].GetY(), corners[3].GetZ());
    glVertex3f(corners[0].GetX(), corners[0].GetY(), corners[0].GetZ());
    // Top
    glVertex3f(corners[4].GetX(), corners[4].GetY(), corners[4].GetZ());
    glVertex3f(corners[5].GetX(), corners[5].GetY(), corners[5].GetZ());
    glVertex3f(corners[5].GetX(), corners[5].GetY(), corners[5].GetZ());
    glVertex3f(corners[6].GetX(), corners[6].GetY(), corners[6].GetZ());
    glVertex3f(corners[6].GetX(), corners[6].GetY(), corners[6].GetZ());
    glVertex3f(corners[7].GetX(), corners[7].GetY(), corners[7].GetZ());
    glVertex3f(corners[7].GetX(), corners[7].GetY(), corners[7].GetZ());
    glVertex3f(corners[4].GetX(), corners[4].GetY(), corners[4].GetZ());
    // Verticals
    glVertex3f(corners[0].GetX(), corners[0].GetY(), corners[0].GetZ());
    glVertex3f(corners[4].GetX(), corners[4].GetY(), corners[4].GetZ());
    glVertex3f(corners[1].GetX(), corners[1].GetY(), corners[1].GetZ());
    glVertex3f(corners[5].GetX(), corners[5].GetY(), corners[5].GetZ());
    glVertex3f(corners[2].GetX(), corners[2].GetY(), corners[2].GetZ());
    glVertex3f(corners[6].GetX(), corners[6].GetY(), corners[6].GetZ());
    glVertex3f(corners[3].GetX(), corners[3].GetY(), corners[3].GetZ());
    glVertex3f(corners[7].GetX(), corners[7].GetY(), corners[7].GetZ());
    glEnd();
}

void DrawCylinder(float radius, float halfHeight, RMat44 transform)
{
    const int segments = 16;

    glBegin(GL_LINES);
    for (int i = 0; i < segments; i++)
    {
        float angle1 = (float)i / segments * 2.0f * 3.14159f;
        float angle2 = (float)(i + 1) / segments * 2.0f * 3.14159f;

        // Circle at +Y (rotated to be wheel axis)
        Vec3 p1 = transform * Vec3(cos(angle1) * radius, halfHeight, sin(angle1) * radius);
        Vec3 p2 = transform * Vec3(cos(angle2) * radius, halfHeight, sin(angle2) * radius);
        glVertex3f(p1.GetX(), p1.GetY(), p1.GetZ());
        glVertex3f(p2.GetX(), p2.GetY(), p2.GetZ());

        // Circle at -Y
        Vec3 p3 = transform * Vec3(cos(angle1) * radius, -halfHeight, sin(angle1) * radius);
        Vec3 p4 = transform * Vec3(cos(angle2) * radius, -halfHeight, sin(angle2) * radius);
        glVertex3f(p3.GetX(), p3.GetY(), p3.GetZ());
        glVertex3f(p4.GetX(), p4.GetY(), p4.GetZ());

        // Connect circles (every 4th segment for spokes)
        if (i % 4 == 0)
        {
            glVertex3f(p1.GetX(), p1.GetY(), p1.GetZ());
            glVertex3f(p3.GetX(), p3.GetY(), p3.GetZ());
        }
    }
    glEnd();
}

void DrawGround()
{
    glColor3f(0.3f, 0.5f, 0.3f);
    glBegin(GL_LINES);
    for (int i = -50; i <= 50; i += 5)
    {
        glVertex3f((float)i, 0, -50);
        glVertex3f((float)i, 0, 50);
        glVertex3f(-50, 0, (float)i);
        glVertex3f(50, 0, (float)i);
    }
    glEnd();
}

void DrawVehicle()
{
    if (!gVehicleConstraint) return;

    BodyInterface& bodyInterface = gPhysicsSystem->GetBodyInterface();

    // Get vehicle body transform
    RMat44 bodyTransform = bodyInterface.GetWorldTransform(gVehicleBodyID);

    // Draw car body
    glColor3f(0.8f, 0.2f, 0.2f);
    DrawBox(Vec3(0.9f, 0.5f, 2.0f), bodyTransform);

    // Draw wheels
    glColor3f(0.2f, 0.2f, 0.8f);
    const Wheels& wheels = gVehicleConstraint->GetWheels();
    for (uint i = 0; i < wheels.size(); i++)
    {
        const Wheel* wheel = wheels[i];
        RMat44 wheelTransform = gVehicleConstraint->GetWheelWorldTransform(i, Vec3::sAxisY(), Vec3::sAxisX());

        // Wheel is oriented with Y as axle, so we need to rotate for drawing
        float radius = wheel->GetSettings()->mRadius;
        float halfWidth = wheel->GetSettings()->mWidth * 0.5f;

        // Draw as cylinder rotated 90 degrees around Z
        DrawCylinder(radius, halfWidth, wheelTransform);
    }
}

void Render(SDL_Window* window)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Get vehicle position for camera tracking
    RVec3 vehiclePos(0, 0, 0);
    if (gVehicleConstraint)
    {
        BodyInterface& bodyInterface = gPhysicsSystem->GetBodyInterface();
        vehiclePos = bodyInterface.GetPosition(gVehicleBodyID);
    }

    // Camera follows vehicle
    float camX = vehiclePos.GetX() + gCameraDistance * sin(gCameraAngle);
    float camY = vehiclePos.GetY() + gCameraHeight;
    float camZ = vehiclePos.GetZ() + gCameraDistance * cos(gCameraAngle);

    gluLookAt(camX, camY, camZ,
              vehiclePos.GetX(), vehiclePos.GetY() + 1, vehiclePos.GetZ(),
              0, 1, 0);

    DrawGround();
    DrawVehicle();

    SDL_GL_SwapWindow(window);
}

void CleanupPhysics()
{
    // Remove vehicle
    if (gVehicleConstraint)
    {
        gPhysicsSystem->RemoveStepListener(gVehicleConstraint);
        gPhysicsSystem->RemoveConstraint(gVehicleConstraint);
    }

    // Cleanup in reverse order
    delete gPhysicsSystem;
    delete gBodyActivationListener;
    delete gContactListener;
    delete gObjectLayerPairFilter;
    delete gObjectVsBroadPhaseLayerFilter;
    delete gBroadPhaseLayerInterface;
    delete gJobSystem;
    delete gTempAllocator;

    UnregisterTypes();
    delete Factory::sInstance;
    Factory::sInstance = nullptr;
}

int main(int argc, char* argv[])
{
    std::cout << "Jolt Vehicle Demo" << std::endl;
    std::cout << "Controls: WASD = Drive, Space = Brake, Q/E = Rotate camera, +/- = Zoom, R = Reset" << std::endl;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Jolt Vehicle Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    if (!window)
    {
        std::cerr << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create OpenGL context
    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext)
    {
        std::cerr << "OpenGL context creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Setup OpenGL
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, 1280.0 / 720.0, 0.1, 1000.0);

    // Initialize physics
    InitPhysics();
    CreateGround();
    CreateVehicle();

    // Main loop
    bool running = true;
    Uint32 lastTime = SDL_GetTicks();

    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
            {
                bool pressed = (event.type == SDL_KEYDOWN);
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                case SDLK_w:
                    gKeyForward = pressed;
                    if (pressed) std::cout << "W pressed" << std::endl;
                    break;
                case SDLK_s:
                    gKeyBackward = pressed;
                    break;
                case SDLK_a:
                    gKeyLeft = pressed;
                    break;
                case SDLK_d:
                    gKeyRight = pressed;
                    break;
                case SDLK_SPACE:
                    gKeyBrake = pressed;
                    break;
                case SDLK_q:
                    if (pressed) gCameraAngle -= 0.2f;
                    break;
                case SDLK_e:
                    if (pressed) gCameraAngle += 0.2f;
                    break;
                case SDLK_r:
                    // Reset vehicle
                    if (pressed && gVehicleConstraint)
                    {
                        BodyInterface& bodyInterface = gPhysicsSystem->GetBodyInterface();
                        bodyInterface.SetPositionAndRotation(gVehicleBodyID,
                            RVec3(0, 2, 0), Quat::sIdentity(), EActivation::Activate);
                        bodyInterface.SetLinearVelocity(gVehicleBodyID, Vec3::sZero());
                        bodyInterface.SetAngularVelocity(gVehicleBodyID, Vec3::sZero());
                    }
                    break;
                case SDLK_EQUALS:
                case SDLK_PLUS:
                case SDLK_KP_PLUS:
                    if (pressed) gCameraDistance = std::max(3.0f, gCameraDistance - 2.0f);
                    break;
                case SDLK_MINUS:
                case SDLK_KP_MINUS:
                    if (pressed) gCameraDistance = std::min(50.0f, gCameraDistance + 2.0f);
                    break;
                case SDLK_UP:
                    if (pressed) gCameraHeight = std::min(20.0f, gCameraHeight + 1.0f);
                    break;
                case SDLK_DOWN:
                    if (pressed) gCameraHeight = std::max(1.0f, gCameraHeight - 1.0f);
                    break;
                }
            }
        }

        // Calculate delta time
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        // Clamp delta time
        if (deltaTime > 0.1f) deltaTime = 0.1f;

        // Update
        UpdateVehicleInput();
        StepPhysics(deltaTime);

        // Render
        Render(window);

        // Cap at ~60 FPS
        SDL_Delay(16);
    }

    // Cleanup
    CleanupPhysics();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
