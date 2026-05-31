#include "SpaceStation.h"
#include "MissionManager.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "../Spacecraft/SpacecraftPawn.h"

ASpaceStation::ASpaceStation()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup    = TG_PrePhysics;

    // Root scene
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    // Visual mesh
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
    MeshComponent->SetupAttachment(RootComponent);
    MeshComponent->SetSimulatePhysics(false);
    MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    MeshComponent->SetEnableGravity(false);

    // Station hull collider – large box, overlap = FAILED
    StationCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("StationCollider"));
    StationCollider->SetupAttachment(RootComponent);
    StationCollider->SetBoxExtent(StationColliderExtent);
    StationCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    StationCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
    StationCollider->SetGenerateOverlapEvents(true);
    StationCollider->ShapeColor = FColor::Red;

    // Docking port collider – small sphere, overlap + low speed = SUCCESS
    DockingCollider = CreateDefaultSubobject<USphereComponent>(TEXT("DockingCollider"));
    DockingCollider->SetupAttachment(RootComponent);
    DockingCollider->SetSphereRadius(DockingColliderRadius);
    DockingCollider->SetRelativeLocation(DockingPortOffset);
    DockingCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    DockingCollider->SetCollisionResponseToAllChannels(ECR_Overlap);
    DockingCollider->SetGenerateOverlapEvents(true);
    DockingCollider->ShapeColor = FColor::Green;
}

void ASpaceStation::BeginPlay()
{
    Super::BeginPlay();

    PhysBody.mass           = Mass;
    PhysBody.state.position = ToPhysics(GetActorLocation());
    PhysBody.state.velocity = Vector3(0, 0, 0);

    // Bind overlap callbacks
    StationCollider->OnComponentBeginOverlap.AddDynamic(
        this, &ASpaceStation::OnStationOverlapBegin);
    DockingCollider->OnComponentBeginOverlap.AddDynamic(
        this, &ASpaceStation::OnDockingOverlapBegin);

    // Auto-find GravityController and init orbit
    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGravityController::StaticClass(), Found);
    if (Found.Num() > 0)
    {
        GravitySource = Cast<AGravityController>(Found[0]);
        InitOrbit(GetActorLocation(),
                  GravitySource->GetActorLocation(),
                  GravitySource->SourceMass,
                  GravitySource->GravitationalConstant);

        // Register with GravityController so gravity is applied each tick
        GravitySource->RegisterSpaceStation(this, false);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("ASpaceStation: no AGravityController found."));
    }
}

void ASpaceStation::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    PhysBody.Update(static_cast<double>(DeltaTime));
    SetActorLocation(ToUnreal(PhysBody.state.position));
}

// ── Overlap callbacks ─────────────────────────────────────────────────────────

void ASpaceStation::OnDockingOverlapBegin(UPrimitiveComponent*, AActor* OtherActor,
                                           UPrimitiveComponent*, int32, bool,
                                           const FHitResult&)
{
    ASpacecraftPawn* SC = Cast<ASpacecraftPawn>(OtherActor);
    if (!SC) return;

    const double RelSpeed = GetRelativeSpeed(SC->GetPhysicsVelocity());

    // Find MissionManager
    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMissionManager::StaticClass(), Found);
    if (Found.Num() == 0) return;
    AMissionManager* MM = Cast<AMissionManager>(Found[0]);
    if (!MM || MM->IsMissionOver()) return;

    if (RelSpeed <= MaxDockingSpeed)
    {
        MM->ReportSuccess(FString::Printf(
            TEXT("Docked at %.2f m/s relative speed"), RelSpeed));
    }
    else
    {
        MM->ReportFailure(FString::Printf(
            TEXT("Too fast at docking port: %.2f m/s (max %.1f)"),
            RelSpeed, MaxDockingSpeed));
    }
}

void ASpaceStation::OnStationOverlapBegin(UPrimitiveComponent*, AActor* OtherActor,
                                           UPrimitiveComponent*, int32, bool,
                                           const FHitResult&)
{
    ASpacecraftPawn* SC = Cast<ASpacecraftPawn>(OtherActor);
    if (!SC) return;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMissionManager::StaticClass(), Found);
    if (Found.Num() == 0) return;
    AMissionManager* MM = Cast<AMissionManager>(Found[0]);
    if (!MM || MM->IsMissionOver()) return;

    const double RelSpeed = GetRelativeSpeed(SC->GetPhysicsVelocity());
    MM->ReportFailure(FString::Printf(
        TEXT("Collision with station hull at %.2f m/s"), RelSpeed));
}

// ── GravityController interface ───────────────────────────────────────────────

void ASpaceStation::ApplyGravityForce(const FVector& Force)
{
    PhysBody.AddForce(Vector3(Force.X, Force.Y, Force.Z));
}

FVector ASpaceStation::GetPhysicsPosition() const
{
    return ToUnreal(PhysBody.state.position);
}

FVector ASpaceStation::GetPhysicsVelocity() const
{
    return ToUnreal(PhysBody.state.velocity);
}

void ASpaceStation::InitOrbit(const FVector& StartPosition, const FVector& SourcePosition,
                               double SourceMass, double GravitationalConstant)
{
    PhysBody.mass           = Mass;
    PhysBody.state.position = ToPhysics(StartPosition);

    const Vector3 diff = ToPhysics(StartPosition) - ToPhysics(SourcePosition);
    const double  r    = diff.Length();
    if (r < 1e-6) return;

    const double  v      = FMath::Sqrt(GravitationalConstant * SourceMass / r);
    const Vector3 radial = diff.Normalized();
    const Vector3 up(0, 0, 1);
    Vector3 tangent(up.y*radial.z - up.z*radial.y,
                    up.z*radial.x - up.x*radial.z,
                    up.x*radial.y - up.y*radial.x);
    if (tangent.Length() < 1e-6)
    {
        Vector3 alt(1, 0, 0);
        tangent = Vector3(alt.y*radial.z - alt.z*radial.y,
                          alt.z*radial.x - alt.x*radial.z,
                          alt.x*radial.y - alt.y*radial.x);
    }
    PhysBody.state.velocity = tangent.Normalized() * v;
    UE_LOG(LogTemp, Log, TEXT("ASpaceStation: orbit r=%.1f m, v=%.2f m/s"), r, v);
}

double ASpaceStation::GetRelativeSpeed(const FVector& SpacecraftVelocityCmS) const
{
    // Both velocities are in cm/s (UE units) – convert to m/s for comparison
    const FVector StationVel = ToUnreal(PhysBody.state.velocity);
    return (SpacecraftVelocityCmS - StationVel).Size() * CM_TO_M;
}

Vector3 ASpaceStation::ToPhysics(const FVector& v)
{ return Vector3(v.X*CM_TO_M, v.Y*CM_TO_M, v.Z*CM_TO_M); }

FVector ASpaceStation::ToUnreal(const Vector3& v)
{ return FVector(v.x*M_TO_CM, v.y*M_TO_CM, v.z*M_TO_CM); }
