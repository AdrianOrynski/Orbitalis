#include "SpacecraftHUD.h"
#include "../Mission/MissionManager.h"
#include "../Mission/SpaceStation.h"
#include "SpacecraftPawn.h"
#include "ThrusterSystem.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Kismet/GameplayStatics.h"

void ASpacecraftHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;

    ASpacecraftPawn* Pawn = Cast<ASpacecraftPawn>(GetOwningPawn());
    if (!Pawn) return;

    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;

    const FLinearColor CText = FLinearColor(0.90f, 0.90f, 0.90f, 1.0f);
    const FLinearColor CBlue = FLinearColor(0.20f, 0.80f, 1.00f, 1.0f);
    const FLinearColor CGreen = FLinearColor(0.20f, 1.00f, 0.30f, 1.0f);
    const FLinearColor CRed = FLinearColor(1.00f, 0.30f, 0.10f, 1.0f);
    const FLinearColor CYellow = FLinearColor(1.00f, 0.80f, 0.00f, 1.0f);
    const FLinearColor COrange = FLinearColor(1.00f, 0.55f, 0.00f, 1.0f);
    const FLinearColor CBg = FLinearColor(0.00f, 0.00f, 0.00f, 0.55f);
    const FLinearColor CDim = FLinearColor(0.20f, 0.20f, 0.20f, 0.70f);
    const FLinearColor COn = FLinearColor(1.00f, 0.55f, 0.05f, 1.0f);  // firing
    const FLinearColor CMain = FLinearColor(0.00f, 0.80f, 1.00f, 1.0f);  // main engine

    // ================================================================
    // LEFT – telemetry
    // ================================================================
    const float PX = 28.0f, PY = 28.0f, PW = 270.0f, LH = 24.0f, Pad = 10.0f;

    const double SpeedMS = Pawn->GetSpeedMS();
    const double AltM = Pawn->GetAltitudeM();
    const double FuelFrac = Pawn->GetFuelFraction();
    const double DeltaV = Pawn->GetDeltaVRemaining();
    const FVector AngVel = Pawn->GetAngularVelocityDegPerSec();
    const bool bLow = FuelFrac < 0.2;

    DrawRect(CBg, PX - Pad, PY - Pad, PW + Pad * 2, LH * 13 + Pad * 2);

    float Y = PY;
    DrawText(TEXT("── ORBITALIS ──"), CText, PX, Y, GEngine->GetSmallFont(), 1.3f); Y += LH * 1.5f;

    DrawText(TEXT("SPEED"), CText, PX, Y, GEngine->GetSmallFont(), 0.9f); Y += LH * 0.8f;
    DrawText(FString::Printf(TEXT("%.2f m/s"), SpeedMS),
        CBlue, PX, Y, GEngine->GetSmallFont(), 1.2f); Y += LH * 1.3f;

    DrawText(TEXT("ALTITUDE"), CText, PX, Y, GEngine->GetSmallFont(), 0.9f); Y += LH * 0.8f;
    DrawText(AltM >= 1000.0
        ? FString::Printf(TEXT("%.2f km"), AltM / 1000.0)
        : FString::Printf(TEXT("%.1f m"), AltM),
        CBlue, PX, Y, GEngine->GetSmallFont(), 1.2f); Y += LH * 1.3f;

    DrawText(TEXT("DELTA-V"), CText, PX, Y, GEngine->GetSmallFont(), 0.9f); Y += LH * 0.8f;
    DrawText(FString::Printf(TEXT("%.1f m/s"), DeltaV),
        COrange, PX, Y, GEngine->GetSmallFont(), 1.2f); Y += LH * 1.3f;

    DrawText(TEXT("ROT  p / y / r"), CText, PX, Y, GEngine->GetSmallFont(), 0.9f); Y += LH * 0.8f;
    DrawText(FString::Printf(TEXT("%.1f  %.1f  %.1f  deg/s"),
        FMath::Abs(AngVel.Y), FMath::Abs(AngVel.Z), FMath::Abs(AngVel.X)),
        CBlue, PX, Y, GEngine->GetSmallFont(), 1.1f); Y += LH * 1.3f;

    DrawText(TEXT("FUEL"), bLow ? CYellow : CText, PX, Y, GEngine->GetSmallFont(), 0.9f); Y += LH * 0.8f;
    DrawBar(PX, Y, PW, 12.0f, static_cast<float>(FuelFrac), bLow ? CRed : CGreen); Y += 16.0f;
    DrawText(FString::Printf(TEXT("%.1f %%"), FuelFrac * 100.0),
        bLow ? CRed : CGreen, PX, Y, GEngine->GetSmallFont(), 1.0f); Y += LH;

    if (bLow && FMath::Fmod(GetWorld()->GetTimeSeconds(), 1.0f) < 0.5f)
        DrawText(TEXT("!! LOW FUEL !!"), CYellow, PX, Y, GEngine->GetSmallFont(), 1.1f);

    // ================================================================
    // RIGHT – thruster diagram  (top-view schematic)
    //
    // Layout matches the physical model:
    //
    //              [FRONT cluster]
    //               U   D   L   R
    //
    //  [LEFT]                    [RIGHT]
    //  U D F B    [ MAIN ]    U D F B
    //
    //              [AFT cluster]
    //               U   D   L   R
    //
    // Each cell is a small box: dim = inactive, orange = firing.
    // ================================================================

    const float DX = SW - 310.0f;
    const float DY = 28.0f;
    const float DW = 280.0f;
    const float DH = 290.0f;
    const float BW = 44.0f;
    const float BH = 16.0f;
    const float Gap = 4.0f;

    DrawRect(CBg, DX - Pad, DY - Pad, DW + Pad * 2, DH + Pad * 2);
    DrawText(TEXT("THRUSTERS"), CText, DX, DY, GEngine->GetSmallFont(), 1.0f);

    const FThrusterSystem& TS = Pawn->GetThrusterSystem();

    auto Box = [&](float X, float Y, EThrusterID ID, const TCHAR* Label, FLinearColor ActiveColor)
        {
            const bool bOn = TS.IsActive(ID);
            DrawRect(bOn ? ActiveColor : CDim, X, Y, BW, BH);
            DrawText(FString(Label), bOn ? FLinearColor::Black : CText,
                X + 2, Y + 2, GEngine->GetSmallFont(), 0.70f);
        };

    // Centre of the diagram
    const float CX = DX + DW * 0.5f;
    const float CY = DY + 50.0f;

    // ── FRONT cluster (top row) ──
    const float FY = CY;
    const float FStartX = CX - (BW * 2 + Gap * 1.5f);
    DrawText(TEXT("FRONT"), CText, FStartX, FY - 14, GEngine->GetSmallFont(), 0.8f);
    Box(FStartX, FY, EThrusterID::FRONT_UP, TEXT("UP"), COn);
    Box(FStartX + BW + Gap, FY, EThrusterID::FRONT_DOWN, TEXT("DOWN"), COn);
    Box(FStartX + (BW + Gap) * 2, FY, EThrusterID::FRONT_LEFT, TEXT("LEFT"), COn);
    Box(FStartX + (BW + Gap) * 3, FY, EThrusterID::FRONT_RIGHT, TEXT("RIGHT"), COn);

    // ── MAIN engine (centre) ──
    const float MY = CY + BH + Gap * 4;
    const float MX = CX - BW * 0.5f;
    DrawRect(TS.IsActive(EThrusterID::MAIN) ? CMain : CDim, MX, MY, BW, BH + 4);
    DrawText(TEXT("MAIN"), TS.IsActive(EThrusterID::MAIN) ? FLinearColor::Black : CText,
        MX + 4, MY + 3, GEngine->GetSmallFont(), 0.85f);
    DrawText(TEXT("[SPACE]"), CText, MX - 2, MY + BH + 6, GEngine->GetSmallFont(), 0.70f);

    // ── LEFT cluster ──
    const float LX = DX + 2.0f;
    const float LY = MY - BH * 0.5f;
    DrawText(TEXT("LEFT"), CText, LX, LY - 14, GEngine->GetSmallFont(), 0.8f);
    Box(LX, LY, EThrusterID::LEFT_UP, TEXT("UP"), COn);
    Box(LX + BW + Gap, LY, EThrusterID::LEFT_DOWN, TEXT("DN"), COn);
    Box(LX, LY + BH + Gap, EThrusterID::LEFT_LEFT, TEXT("LEFT"), COn);
    Box(LX + BW + Gap, LY + BH + Gap, EThrusterID::LEFT_RIGHT, TEXT("RIGHT"), COn);

    // ── RIGHT cluster ──
    const float RX = DX + DW - BW * 2 - Gap - 2.0f;
    const float RY = LY;
    DrawText(TEXT("RIGHT"), CText, RX, RY - 14, GEngine->GetSmallFont(), 0.8f);
    Box(RX, RY, EThrusterID::RIGHT_UP, TEXT("UP"), COn);
    Box(RX + BW + Gap, RY, EThrusterID::RIGHT_DOWN, TEXT("DN"), COn);
    Box(RX, RY + BH + Gap, EThrusterID::RIGHT_LEFT, TEXT("LEFT"), COn);
    Box(RX + BW + Gap, RY + BH + Gap, EThrusterID::RIGHT_RIGHT, TEXT("RIGHT"), COn);

    // ── AFT cluster (bottom row) ──
    const float AftY = MY + BH + Gap * 5;
    const float AftStartX = FStartX;
    DrawText(TEXT("AFT"), CText, AftStartX, AftY - 14, GEngine->GetSmallFont(), 0.8f);
    Box(AftStartX, AftY, EThrusterID::AFT_UP, TEXT("UP"), COn);
    Box(AftStartX + BW + Gap, AftY, EThrusterID::AFT_DOWN, TEXT("DOWN"), COn);
    Box(AftStartX + (BW + Gap) * 2, AftY, EThrusterID::AFT_LEFT, TEXT("LEFT"), COn);
    Box(AftStartX + (BW + Gap) * 3, AftY, EThrusterID::AFT_RIGHT, TEXT("RIGHT"), COn);

    // ── Key reference under diagram ──
    const float KY = DY + DH - 10.0f;
    DrawText(TEXT("1-4: FRONT  5-8: AFT"), CText, DX, KY, GEngine->GetSmallFont(), 0.85f);
    DrawText(TEXT("Num7/1/4/8: LEFT   Num9/3/6/2: RIGHT"), CText, DX, KY + 16, GEngine->GetSmallFont(), 0.85f);


    // ================================================================
    // MISSION PANEL – distance/speed to station + camera mode hint
    // ================================================================
    TArray<AActor*> StationActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpaceStation::StaticClass(), StationActors);

    TArray<AActor*> MMActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMissionManager::StaticClass(), MMActors);
    AMissionManager* MM = MMActors.Num() > 0 ? Cast<AMissionManager>(MMActors[0]) : nullptr;

    if (StationActors.Num() > 0 && MM && !MM->IsMissionOver())
    {
        ASpaceStation* Station = Cast<ASpaceStation>(StationActors[0]);
        const float MMX = SW * 0.5f - 150.0f;
        const float MMY = 28.0f;
        DrawRect(CBg, MX - Pad, MY - Pad, 300.0f + Pad * 2, LH * 4 + Pad * 2);

        DrawText(TEXT("DOCKING MISSION"), CText, MMX, MMY, GEngine->GetSmallFont(), 1.1f);

        // Distance to docking port
        const FVector DockPos = Station->GetActorLocation()
            + Station->GetActorRotation().RotateVector(Station->DockingPortOffset);
        const double DistM = (Pawn->GetActorLocation() - DockPos).Size() * 0.01;
        DrawText(FString::Printf(TEXT("Distance to dock: %.1f m"), DistM),
            CBlue, MMX, MMY + LH, GEngine->GetSmallFont(), 1.0f);

        // Relative speed
        const double RelSpeed = Station->GetRelativeSpeed(Pawn->GetPhysicsVelocity());
        const FLinearColor SpeedColor = RelSpeed <= 2.0 ? CGreen : (RelSpeed <= 5.0 ? CYellow : CRed);
        DrawText(FString::Printf(TEXT("Relative speed:   %.2f m/s"), RelSpeed),
            SpeedColor, MMX, MMY + LH * 2, GEngine->GetSmallFont(), 1.0f);

        // Timer
        DrawText(FString::Printf(TEXT("Time: %.1f s"), MM->ElapsedTime),
            CText, MMX, MMY + LH * 3, GEngine->GetSmallFont(), 1.0f);
    }

    // ================================================================
    // MISSION RESULT SCREEN
    // ================================================================
    if (MM && MM->IsMissionOver())
    {
        const bool bSuccess = MM->State == EMissionState::Success;
        const FLinearColor ResultBg = bSuccess
            ? FLinearColor(0.0f, 0.2f, 0.0f, 0.85f)
            : FLinearColor(0.2f, 0.0f, 0.0f, 0.85f);
        const FLinearColor ResultCol = bSuccess ? CGreen : CRed;

        const float RW = 500.0f, RH = 200.0f;
        const float RRX = SW * 0.5f - RW * 0.5f;
        const float RRY = SH * 0.5f - RH * 0.5f;

        DrawRect(ResultBg, RRX, RRY, RW, RH);

        // Title
        const FString Title = bSuccess ? TEXT("✓  DOCKING SUCCESS") : TEXT("✗  MISSION FAILED");
        DrawText(Title, ResultCol, RRX + 20.0f, RRY + 20.0f, GEngine->GetSmallFont(), 1.8f);

        // Detail lines from ResultMessage
        TArray<FString> Lines;
        MM->ResultMessage.ParseIntoArrayLines(Lines);
        float LineY = RRY + 70.0f;
        for (const FString& Line : Lines)
        {
            if (!Line.IsEmpty())
            {
                DrawText(Line, CText, RRX + 20.0f, LineY, GEngine->GetSmallFont(), 1.1f);
                LineY += 26.0f;
            }
        }

        // Restart hint
        DrawText(TEXT("Press  R  to restart"), CYellow,
            RRX + 20.0f, RRY + RH - 30.0f, GEngine->GetSmallFont(), 1.0f);

        return;  // skip rest of HUD when mission is over
    }

    // ================================================================
    // BOTTOM LEFT – controls
    // ================================================================
    const float HX = 28.0f, HY = SH - 110.0f;
    DrawRect(CBg, HX - Pad, HY - Pad, 240.0f, 100.0f);
    DrawText(TEXT("CONTROLS"), CText, HX, HY, GEngine->GetSmallFont(), 1.0f);
    DrawText(TEXT("Space   – main engine"), CText, HX, HY + 24, GEngine->GetSmallFont(), 1.0f);
    DrawText(TEXT("1-8     – FRONT/AFT clusters"), CText, HX, HY + 46, GEngine->GetSmallFont(), 1.0f);
    DrawText(TEXT("Numpad  – LEFT/RIGHT clusters"), CText, HX, HY + 68, GEngine->GetSmallFont(), 1.0f);
}

void ASpacecraftHUD::DrawBar(float X, float Y, float W, float H,
    float Fraction, FLinearColor Fill)
{
    DrawRect(FLinearColor(0.15f, 0.15f, 0.15f, 0.8f), X, Y, W, H);
    if (Fraction > 0.0f)
        DrawRect(Fill, X, Y, W * FMath::Clamp(Fraction, 0.0f, 1.0f), H);
}