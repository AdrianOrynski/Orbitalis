#include "SpacecraftPawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"
#include "../Mission/SpaceStation.h"
#include "EnhancedInputSubsystems.h"
#include <cmath>

ASpacecraftPawn::ASpacecraftPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;

    // SceneRoot – plain root, never moved by physics code
    USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    // MeshComponent attached to root – rotate freely in Blueprint
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    MeshComponent->SetEnableGravity(false);

    // SpringArm attached to MeshComponent – follows mesh orientation
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(MeshComponent);
    SpringArm->TargetArmLength = 600.0f;
    SpringArm->bUsePawnControlRotation = false;
    SpringArm->bInheritPitch = true;
    SpringArm->bInheritYaw = true;
    SpringArm->bInheritRoll = true;

    // Camera at the end of SpringArm
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;

    // Spacecraft collision sphere
    SpacecraftCollider = CreateDefaultSubobject<USphereComponent>(TEXT("SpacecraftCollider"));
    SpacecraftCollider->SetupAttachment(RootComponent);
    SpacecraftCollider->SetSphereRadius(80.0f);
    SpacecraftCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SpacecraftCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
    SpacecraftCollider->SetGenerateOverlapEvents(true);
}


void ASpacecraftPawn::BeginPlay()
{
    Super::BeginPlay();

    CurrentFuel = FuelMass;
    PhysBody.mass = Mass;
    PhysBody.state.position = ToPhysics(GetActorLocation());
    PhysBody.state.velocity = Vector3(0.0, 0.0, 0.0);

    Thrusters.MainEngineForce = MainEngineForce;
    Thrusters.RcsForce = RcsForce;

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Sub =
            ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (SpacecraftMappingContext)
                Sub->AddMappingContext(SpacecraftMappingContext, 0);
            else
                UE_LOG(LogTemp, Warning, TEXT("SpacecraftPawn: SpacecraftMappingContext not set!"));
        }
    }

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGravityController::StaticClass(), Found);
    if (Found.Num() > 0)
    {
        GravitySource = Cast<AGravityController>(Found[0]);
        GravitySourcePosition = GravitySource->GetActorLocation();
        InitOrbit(GetActorLocation(), GravitySource->GetActorLocation(),
            GravitySource->SourceMass, GravitySource->GravitationalConstant);
        GravitySource->RegisterSpacecraft(this, false);
        AddTickPrerequisiteActor(GravitySource);
    }

    TArray<AActor*> FoundStations;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpaceStation::StaticClass(), FoundStations);
    if (FoundStations.Num() > 0)
        TargetStation = Cast<ASpaceStation>(FoundStations[0]);
}

void ASpacecraftPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!HasActorBegunPlay()) return;

    ApplyThrusterForces(DeltaTime);
    IntegrateAngularVelocity(DeltaTime);
    PhysBody.Update(static_cast<double>(DeltaTime));
    SetActorLocation(ToUnreal(PhysBody.state.position));
    Thrusters.ResetActiveThrusters();

    if (IsValid(GravitySource))
        GravitySourcePosition = GravitySource->GetActorLocation();

    TickCameraMode();
}

void ASpacecraftPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (!EIC) return;

    // Macro to reduce boilerplate
    auto Bind = [&](UInputAction* IA, auto Fn)
        {
            if (IA) EIC->BindAction(IA, ETriggerEvent::Triggered, this, Fn);
        };

    Bind(IA_Main, &ASpacecraftPawn::OnMain);
    Bind(IA_Front_Up, &ASpacecraftPawn::OnFrontUp);
    Bind(IA_Front_Down, &ASpacecraftPawn::OnFrontDown);
    Bind(IA_Front_Left, &ASpacecraftPawn::OnFrontLeft);
    Bind(IA_Front_Right, &ASpacecraftPawn::OnFrontRight);
    Bind(IA_Aft_Up, &ASpacecraftPawn::OnAftUp);
    Bind(IA_Aft_Down, &ASpacecraftPawn::OnAftDown);
    Bind(IA_Aft_Left, &ASpacecraftPawn::OnAftLeft);
    Bind(IA_Aft_Right, &ASpacecraftPawn::OnAftRight);
    Bind(IA_Left_Up, &ASpacecraftPawn::OnLeftUp);
    Bind(IA_Left_Down, &ASpacecraftPawn::OnLeftDown);
    Bind(IA_Left_Left, &ASpacecraftPawn::OnLeftLeft);
    Bind(IA_Left_Right, &ASpacecraftPawn::OnLeftRight);
    Bind(IA_Right_Up, &ASpacecraftPawn::OnRightUp);
    Bind(IA_Right_Down, &ASpacecraftPawn::OnRightDown);
    Bind(IA_Right_Left, &ASpacecraftPawn::OnRightLeft);
    Bind(IA_Right_Right, &ASpacecraftPawn::OnRightRight);

    Bind(IA_Cam_Chase,    &ASpacecraftPawn::OnCamChase);
    Bind(IA_Cam_Overview, &ASpacecraftPawn::OnCamOverview);
    Bind(IA_Cam_Docking,  &ASpacecraftPawn::OnCamDocking);
}

// ── Input callbacks ──────────────────────────────────────────────────────────

void ASpacecraftPawn::OnMain(const FInputActionValue&) { Thrusters.Fire(EThrusterID::MAIN); }
void ASpacecraftPawn::OnFrontUp(const FInputActionValue&) { Thrusters.Fire(EThrusterID::FRONT_UP); }
void ASpacecraftPawn::OnFrontDown(const FInputActionValue&) { Thrusters.Fire(EThrusterID::FRONT_DOWN); }
void ASpacecraftPawn::OnFrontLeft(const FInputActionValue&) { Thrusters.Fire(EThrusterID::FRONT_LEFT); }
void ASpacecraftPawn::OnFrontRight(const FInputActionValue&) { Thrusters.Fire(EThrusterID::FRONT_RIGHT); }
void ASpacecraftPawn::OnAftUp(const FInputActionValue&) { Thrusters.Fire(EThrusterID::AFT_UP); }
void ASpacecraftPawn::OnAftDown(const FInputActionValue&) { Thrusters.Fire(EThrusterID::AFT_DOWN); }
void ASpacecraftPawn::OnAftLeft(const FInputActionValue&) { Thrusters.Fire(EThrusterID::AFT_LEFT); }
void ASpacecraftPawn::OnAftRight(const FInputActionValue&) { Thrusters.Fire(EThrusterID::AFT_RIGHT); }
void ASpacecraftPawn::OnLeftUp(const FInputActionValue&) { Thrusters.Fire(EThrusterID::LEFT_UP); }
void ASpacecraftPawn::OnLeftDown(const FInputActionValue&) { Thrusters.Fire(EThrusterID::LEFT_DOWN); }
void ASpacecraftPawn::OnLeftLeft(const FInputActionValue&) { Thrusters.Fire(EThrusterID::LEFT_LEFT); }
void ASpacecraftPawn::OnLeftRight(const FInputActionValue&) { Thrusters.Fire(EThrusterID::LEFT_RIGHT); }
void ASpacecraftPawn::OnRightUp(const FInputActionValue&) { Thrusters.Fire(EThrusterID::RIGHT_UP); }
void ASpacecraftPawn::OnRightDown(const FInputActionValue&) { Thrusters.Fire(EThrusterID::RIGHT_DOWN); }
void ASpacecraftPawn::OnRightLeft(const FInputActionValue&) { Thrusters.Fire(EThrusterID::RIGHT_LEFT); }
void ASpacecraftPawn::OnRightRight(const FInputActionValue&) { Thrusters.Fire(EThrusterID::RIGHT_RIGHT); }

// ── Physics ──────────────────────────────────────────────────────────────────

void ASpacecraftPawn::ApplyThrusterForces(float DeltaTime)
{
    const FThrusterFireResult Result = Thrusters.ComputeForces();

    if (Result.LinearForce.Length() > 1e-6 && CurrentFuel > 0.0)
    {
        const FVector LocalF(Result.LinearForce.x, Result.LinearForce.y, Result.LinearForce.z);
        const FVector WorldF = GetActorRotation().RotateVector(LocalF);
        PhysBody.AddForce(Vector3(WorldF.X, WorldF.Y, WorldF.Z));
        ConsumeFuel(Result.LinearForce.Length(), DeltaTime);
    }

    if (Result.Torque.Length() > 1e-6 && CurrentFuel > 0.0)
    {
        const FVector Alpha(
            Result.Torque.x / MomentOfInertia,
            Result.Torque.y / MomentOfInertia,
            Result.Torque.z / MomentOfInertia
        );
        AngularVelocityRadPerSec += Alpha * DeltaTime;
        AngularVelocityRadPerSec *= FMath::Pow(0.98f, DeltaTime * 60.0f);
    }
}

void ASpacecraftPawn::IntegrateAngularVelocity(float DeltaTime)
{
    if (AngularVelocityRadPerSec.IsNearlyZero(1e-6f)) return;
    const FVector DeltaDeg = AngularVelocityRadPerSec * FMath::RadiansToDegrees(1.0f) * DeltaTime;
    AddActorLocalRotation(FRotator(DeltaDeg.Y, DeltaDeg.Z, DeltaDeg.X));
}

void ASpacecraftPawn::ApplyGravityForce(const FVector& Force)
{
    PhysBody.AddForce(Vector3(Force.X, Force.Y, Force.Z));
}

FVector ASpacecraftPawn::GetPhysicsPosition() const { return ToUnreal(PhysBody.state.position); }
FVector ASpacecraftPawn::GetPhysicsVelocity() const { return ToUnreal(PhysBody.state.velocity); }

void ASpacecraftPawn::InitOrbit(const FVector& StartPosition, const FVector& SourcePosition,
    double SourceMass, double GravitationalConstant)
{
    PhysBody.mass = Mass;
    PhysBody.state.position = ToPhysics(StartPosition);

    const Vector3 diff = ToPhysics(StartPosition) - ToPhysics(SourcePosition);
    const double  r = diff.Length();
    if (r < 1e-6) return;

    const double  v = FMath::Sqrt(GravitationalConstant * SourceMass / r);
    const Vector3 radial = diff.Normalized();
    const Vector3 up = Vector3(0, 0, 1);
    Vector3 tangent(up.y * radial.z - up.z * radial.y,
        up.z * radial.x - up.x * radial.z,
        up.x * radial.y - up.y * radial.x);
    if (tangent.Length() < 1e-6)
    {
        Vector3 alt(1, 0, 0);
        tangent = Vector3(alt.y * radial.z - alt.z * radial.y,
            alt.z * radial.x - alt.x * radial.z,
            alt.x * radial.y - alt.y * radial.x);
    }
    PhysBody.state.velocity = tangent.Normalized() * v;
    UE_LOG(LogTemp, Log, TEXT("SpacecraftPawn: orbit r=%.1f m, v=%.2f m/s"), r, v);
}

double ASpacecraftPawn::GetSpeedMS()      const { return PhysBody.state.velocity.Length(); }
double ASpacecraftPawn::GetAltitudeM()    const { return (ToUnreal(PhysBody.state.position) - GravitySourcePosition).Size() * CM_TO_M; }
double ASpacecraftPawn::GetFuelFraction() const { return FuelMass > 0.0 ? FMath::Clamp(CurrentFuel / FuelMass, 0.0, 1.0) : 1.0; }

double ASpacecraftPawn::GetDeltaVRemaining() const
{
    const double m_total = PhysBody.mass;
    const double m_dry = FMath::Max(m_total - CurrentFuel, 1.0);
    if (m_dry >= m_total) return 0.0;
    return Isp * g0 * FMath::Loge(m_total / m_dry);
}

FVector ASpacecraftPawn::GetAngularVelocityDegPerSec() const
{
    return AngularVelocityRadPerSec * FMath::RadiansToDegrees(1.0f);
}

void ASpacecraftPawn::ConsumeFuel(double TotalForce, float DeltaTime)
{
    CurrentFuel = FMath::Max(CurrentFuel - TotalForce * FuelConsumptionRate * DeltaTime, 0.0);
    PhysBody.mass = FMath::Max(Mass - (FuelMass - CurrentFuel), 1.0);
}


// ── Camera selection ──────────────────────────────────────────────────────────

void ASpacecraftPawn::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
    if (Camera)
    {
        Camera->GetCameraView(DeltaTime, OutResult);
        return;
    }
    Super::CalcCamera(DeltaTime, OutResult);
}

// ── Camera mode callbacks ─────────────────────────────────────────────────────

void ASpacecraftPawn::OnCamChase(const FInputActionValue&)
{
    CurrentCameraMode = ECameraMode::Chase;
    UpdateCameraMode();
}

void ASpacecraftPawn::OnCamOverview(const FInputActionValue&)
{
    CurrentCameraMode = ECameraMode::Overview;
    UpdateCameraMode();
}

void ASpacecraftPawn::OnCamDocking(const FInputActionValue&)
{
    CurrentCameraMode = ECameraMode::Docking;
    UpdateCameraMode();
}

void ASpacecraftPawn::UpdateCameraMode()
{
    TickCameraMode();
}

void ASpacecraftPawn::TickCameraMode()
{
    switch (CurrentCameraMode)
    {
    case ECameraMode::Chase:
    {
        SpringArm->SetRelativeLocation(FVector::ZeroVector);
        SpringArm->TargetArmLength = 600.0f;
        const FVector WorldVel = ToUnreal(PhysBody.state.velocity);
        if (!WorldVel.IsNearlyZero(1.0f))
            SpringArm->SetWorldRotation((-WorldVel.GetSafeNormal()).Rotation());
        break;
    }

    case ECameraMode::Overview:
    {
        const FVector TargetPos = IsValid(TargetStation)
            ? TargetStation->GetActorLocation()
            : GravitySourcePosition;
        const FVector ToTarget = (TargetPos - GetActorLocation()).GetSafeNormal();
        const float   DistCm   = FVector::Dist(TargetPos, GetActorLocation());
        SpringArm->SetRelativeLocation(FVector::ZeroVector);
        SpringArm->SetWorldRotation((-ToTarget).Rotation());
        SpringArm->TargetArmLength = FMath::Max(DistCm * 0.5f, 2000.0f);
        break;
    }

    case ECameraMode::Docking:
    {
        const FVector TargetPos  = IsValid(TargetStation)
            ? TargetStation->GetActorLocation()
            : GravitySourcePosition;
        SpringArm->SetRelativeLocation(FVector(300.0f, 0.0f, 0.0f));
        SpringArm->TargetArmLength = 0.0f;
        const FVector NoseWorld  = GetActorLocation()
            + GetActorRotation().RotateVector(FVector(300.0f, 0.0f, 0.0f));
        const FVector ToTarget   = (TargetPos - NoseWorld).GetSafeNormal();
        if (!ToTarget.IsNearlyZero())
            SpringArm->SetWorldRotation(ToTarget.Rotation());
        break;
    }
    }
}

Vector3 ASpacecraftPawn::ToPhysics(const FVector& v) { return Vector3(v.X * CM_TO_M, v.Y * CM_TO_M, v.Z * CM_TO_M); }
FVector ASpacecraftPawn::ToUnreal(const Vector3& v) { return FVector(v.x * M_TO_CM, v.y * M_TO_CM, v.z * M_TO_CM); }