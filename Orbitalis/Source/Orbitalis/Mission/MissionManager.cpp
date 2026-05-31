#include "MissionManager.h"
#include "Kismet/GameplayStatics.h"

AMissionManager::AMissionManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AMissionManager::BeginPlay()
{
    Super::BeginPlay();
}

void AMissionManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (State == EMissionState::InProgress)
        ElapsedTime += DeltaTime;
}

void AMissionManager::ReportSuccess(const FString& Detail)
{
    if (State != EMissionState::InProgress) return;
    State = EMissionState::Success;
    ResultMessage = FString::Printf(TEXT("SUCCESS\n%s\nTime: %.1f s"), *Detail, ElapsedTime);
    UE_LOG(LogTemp, Log, TEXT("Mission SUCCESS: %s"), *Detail);
}

void AMissionManager::ReportFailure(const FString& Detail)
{
    if (State != EMissionState::InProgress) return;
    State = EMissionState::Failed;
    ResultMessage = FString::Printf(TEXT("MISSION FAILED\n%s\nTime: %.1f s"), *Detail, ElapsedTime);
    UE_LOG(LogTemp, Log, TEXT("Mission FAILED: %s"), *Detail);
}

void AMissionManager::RestartMission()
{
    UGameplayStatics::OpenLevel(this,
        FName(*UGameplayStatics::GetCurrentLevelName(this)));
}
