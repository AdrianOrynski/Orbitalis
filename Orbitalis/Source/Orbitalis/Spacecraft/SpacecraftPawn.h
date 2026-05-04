#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "../Physics/PhysicsObject.h"
#include "../Gravity/GravityController.h"
#include "ThrusterSystem.h"
#include "InputActionValue.h"

#include "SpacecraftPawn.generated.h"

class UInputMappingContext;
class UInputAction;
class USpringArmComponent;
class UCameraComponent;

/**
 * ASpacecraftPawn
 *
 * 17-thruster spacecraft:
 *   Space         – MAIN engine
 *   Keys 1–8      – FRONT/AFT cluster nozzles (see below)
 *   Numpad 1–9    – LEFT/RIGHT cluster nozzles (without 5)
 *
 * Key layout:
 *
 *   FRONT cluster:          AFT cluster:
 *     1 = FRONT_UP            5 = (unused – centre numpad)
 *     2 = FRONT_DOWN          
 *     3 = FRONT_LEFT          
 *     4 = FRONT_RIGHT         
 *     5 = AFT_UP              
 *     6 = AFT_DOWN            
 *     7 = AFT_LEFT            
 *     8 = AFT_RIGHT           
 *
 *   Numpad (LEFT cluster):  Numpad (RIGHT cluster):
 *     Num7 = LEFT_UP          Num9  = RIGHT_UP
 *     Num1 = LEFT_DOWN        Num3  = RIGHT_DOWN
 *     Num4 = LEFT_LEFT         Num6  = RIGHT_LEFT
 *     Num8 = LEFT_RIGHT         Num2  = RIGHT_RIGHT (reused)
 *
 *   Pitch / Yaw via mouse (Axis1D) for fine attitude control.
 */
UCLASS(BlueprintType, Blueprintable)
class ORBITALIS_API ASpacecraftPawn : public APawn
{
    GENERATED_BODY()

public:
    ASpacecraftPawn();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    // ── Components ──────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UCameraComponent* Camera;

    // ── Input mapping ───────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    UInputMappingContext* SpacecraftMappingContext;

    // Main engine
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Main")
    UInputAction* IA_Main;           // Space

    // Front cluster  (keys 1-4)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Front")
    UInputAction* IA_Front_Up;       // 1
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Front")
    UInputAction* IA_Front_Down;     // 2
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Front")
    UInputAction* IA_Front_Left;     // 3
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Front")
    UInputAction* IA_Front_Right;    // 4

    // Aft cluster  (keys 5-8)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Aft")
    UInputAction* IA_Aft_Up;         // 5
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Aft")
    UInputAction* IA_Aft_Down;       // 6
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Aft")
    UInputAction* IA_Aft_Left;       // 7
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Aft")
    UInputAction* IA_Aft_Right;      // 8

    // Left cluster  (Numpad 7,1,4,8)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Left")
    UInputAction* IA_Left_Up;        // Num7
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Left")
    UInputAction* IA_Left_Down;      // Num1
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Left")
    UInputAction* IA_Left_Left;       // Num4
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Left")
    UInputAction* IA_Left_Right;       // Num8 (up on numpad = backward)

    // Right cluster  (Numpad 9,3,6,2)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Right")
    UInputAction* IA_Right_Up;       // Num9
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Right")
    UInputAction* IA_Right_Down;     // Num3
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Right")
    UInputAction* IA_Right_Left;      // Num6
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input|Right")
    UInputAction* IA_Right_Right;      // Num2

    // ── Physics ─────────────────────────────────────────────────────────────

    PhysicsObject PhysBody;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft|Physics")
    double Mass = 5.0e4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft|Physics")
    double MomentOfInertia = 8.0e5;

    // ── Thrusters ───────────────────────────────────────────────────────────

    FThrusterSystem Thrusters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft|Thrusters")
    double MainEngineForce = 1.0e7;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft|Thrusters")
    double RcsForce = 5.0e5;

    // ── Fuel ────────────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft|Fuel")
    double FuelMass = 2.0e4;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spacecraft|Fuel")
    double CurrentFuel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacecraft|Fuel")
    double FuelConsumptionRate = 2.0e-7;

    // ── GravityController interface ─────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Spacecraft|Gravity")
    void ApplyGravityForce(const FVector& Force);

    UFUNCTION(BlueprintCallable, Category = "Spacecraft|Gravity")
    FVector GetPhysicsPosition() const;

    UFUNCTION(BlueprintCallable, Category = "Spacecraft|Gravity")
    FVector GetPhysicsVelocity() const;

    void InitOrbit(const FVector& StartPosition, const FVector& SourcePosition,
                   double SourceMass, double GravitationalConstant);

    // ── HUD queries ─────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Spacecraft|HUD")
    double GetSpeedMS() const;

    UFUNCTION(BlueprintCallable, Category = "Spacecraft|HUD")
    double GetAltitudeM() const;

    UFUNCTION(BlueprintCallable, Category = "Spacecraft|HUD")
    double GetFuelFraction() const;

    UFUNCTION(BlueprintCallable, Category = "Spacecraft|HUD")
    double GetDeltaVRemaining() const;

    UFUNCTION(BlueprintCallable, Category = "Spacecraft|HUD")
    FVector GetAngularVelocityDegPerSec() const;

    const FThrusterSystem& GetThrusterSystem() const { return Thrusters; }

private:
    // One callback per IA (all bool/digital)
    void OnMain(const FInputActionValue&);
    void OnFrontUp(const FInputActionValue&);
    void OnFrontDown(const FInputActionValue&);
    void OnFrontLeft(const FInputActionValue&);
    void OnFrontRight(const FInputActionValue&);
    void OnAftUp(const FInputActionValue&);
    void OnAftDown(const FInputActionValue&);
    void OnAftLeft(const FInputActionValue&);
    void OnAftRight(const FInputActionValue&);
    void OnLeftUp(const FInputActionValue&);
    void OnLeftDown(const FInputActionValue&);
    void OnLeftLeft(const FInputActionValue&);
    void OnLeftRight(const FInputActionValue&);
    void OnRightUp(const FInputActionValue&);
    void OnRightDown(const FInputActionValue&);
    void OnRightLeft(const FInputActionValue&);
    void OnRightRight(const FInputActionValue&);

    void ApplyThrusterForces(float DeltaTime);
    void IntegrateAngularVelocity(float DeltaTime);
    void ConsumeFuel(double TotalForce, float DeltaTime);

    static Vector3 ToPhysics(const FVector& v);
    static FVector ToUnreal(const Vector3& v);

    static constexpr double CM_TO_M = 0.01;
    static constexpr double M_TO_CM = 100.0;
    static constexpr double Isp     = 300.0;
    static constexpr double g0      = 9.80665;

    FVector AngularVelocityRadPerSec = FVector::ZeroVector;

    UPROPERTY()
    AGravityController* GravitySource = nullptr;

    FVector GravitySourcePosition = FVector::ZeroVector;
};
