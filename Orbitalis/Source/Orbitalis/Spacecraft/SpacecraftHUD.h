#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SpacecraftHUD.generated.h"

UCLASS()
class ORBITALIS_API ASpacecraftHUD : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;

private:
    void DrawBar(float X, float Y, float W, float H,
                 float Fraction, FLinearColor Fill);
};
