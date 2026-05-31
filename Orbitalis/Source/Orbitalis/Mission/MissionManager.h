#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MissionManager.generated.h"

UENUM(BlueprintType)
enum class EMissionState : uint8
{
    InProgress,
    Success,
    Failed
};

/**
 * AMissionManager
 *
 * Singleton-style actor placed in the level.
 * Tracks mission state, elapsed time and triggers the end-screen.
 *
 * SpaceStation calls ReportSuccess() / ReportFailure() on it.
 * SpacecraftHUD reads State, ElapsedTime and ResultMessage to draw the overlay.
 *
 * To use: place one AMissionManager in the level.
 * SpaceStation finds it automatically via GetAllActorsOfClass.
 */
UCLASS(BlueprintType, Blueprintable)
class ORBITALIS_API AMissionManager : public AActor
{
    GENERATED_BODY()

public:
    AMissionManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── State ────────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission")
    EMissionState State = EMissionState::InProgress;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission")
    float ElapsedTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission")
    FString ResultMessage;

    // ── API called by SpaceStation ───────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void ReportSuccess(const FString& Detail);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    void ReportFailure(const FString& Detail);

    UFUNCTION(BlueprintCallable, Category = "Mission")
    bool IsMissionOver() const { return State != EMissionState::InProgress; }

    /** Reload the current level. */
    UFUNCTION(BlueprintCallable, Category = "Mission")
    void RestartMission();
};
