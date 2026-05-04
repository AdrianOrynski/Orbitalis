#include "SpacecraftHUD.h"
#include "SpacecraftPawn.h"
#include "ThrusterSystem.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"

void ASpacecraftHUD::DrawHUD()
{
    Super::DrawHUD();
    if (!Canvas) return;

    ASpacecraftPawn* Pawn = Cast<ASpacecraftPawn>(GetOwningPawn());
    if (!Pawn) return;

    const float SW = Canvas->SizeX;
    const float SH = Canvas->SizeY;

    const FLinearColor CText   = FLinearColor(0.90f, 0.90f, 0.90f, 1.0f);
    const FLinearColor CBlue   = FLinearColor(0.20f, 0.80f, 1.00f, 1.0f);
    const FLinearColor CGreen  = FLinearColor(0.20f, 1.00f, 0.30f, 1.0f);
    const FLinearColor CRed    = FLinearColor(1.00f, 0.30f, 0.10f, 1.0f);
    const FLinearColor CYellow = FLinearColor(1.00f, 0.80f, 0.00f, 1.0f);
    const FLinearColor COrange = FLinearColor(1.00f, 0.55f, 0.00f, 1.0f);
    const FLinearColor CBg     = FLinearColor(0.00f, 0.00f, 0.00f, 0.55f);
    const FLinearColor CDim    = FLinearColor(0.20f, 0.20f, 0.20f, 0.70f);
    const FLinearColor COn     = FLinearColor(1.00f, 0.55f, 0.05f, 1.0f);  // firing
    const FLinearColor CMain   = FLinearColor(0.00f, 0.80f, 1.00f, 1.0f);  // main engine

    // ================================================================
    // LEFT – telemetry
    // ================================================================
    const float PX = 28.0f, PY = 28.0f, PW = 270.0f, LH = 24.0f, Pad = 10.0f;

    const double SpeedMS  = Pawn->GetSpeedMS();
    const double AltM     = Pawn->GetAltitudeM();
    const double FuelFrac = Pawn->GetFuelFraction();
    const double DeltaV   = Pawn->GetDeltaVRemaining();
    const FVector AngVel  = Pawn->GetAngularVelocityDegPerSec();
    const bool bLow       = FuelFrac < 0.2;

    DrawRect(CBg, PX-Pad, PY-Pad, PW+Pad*2, LH*13+Pad*2);

    float Y = PY;
    DrawText(TEXT("── ORBITALIS ──"), CText, PX, Y, GEngine->GetSmallFont(), 1.3f); Y += LH*1.5f;

    DrawText(TEXT("SPEED"), CText, PX, Y, GEngine->GetSmallFont(), 0.9f); Y += LH*0.8f;
    DrawText(SpeedMS >= 1000.0
        ? FString::Printf(TEXT("%.3f km/s"), SpeedMS/1000.0)
        : FString::Printf(TEXT("%.2f m/s"),  SpeedMS),
        CBlue, PX, Y, GEngine->GetSmallFont(), 1.2f); Y += LH*1.3f;

    DrawText(TEXT("ALTITUDE"), CText, PX, Y, GEngine->GetSmallFont(), 0.9f); Y += LH*0.8f;
    DrawText(AltM >= 1000.0
        ? FString::Printf(TEXT("%.2f km"), AltM/1000.0)
        : FString::Printf(TEXT("%.1f m"),  AltM),
        CBlue, PX, Y, GEngine->GetSmallFont(), 1.2f); Y += LH*1.3f;

    DrawText(TEXT("DELTA-V"), CText, PX, Y, GEngine->GetSmallFont(), 0.9f); Y += LH*0.8f;
    DrawText(FString::Printf(TEXT("%.1f m/s"), DeltaV),
        COrange, PX, Y, GEngine->GetSmallFont(), 1.2f); Y += LH*1.3f;

    DrawText(TEXT("ROT  p / y / r"), CText, PX, Y, GEngine->GetSmallFont(), 0.9f); Y += LH*0.8f;
    DrawText(FString::Printf(TEXT("%.1f  %.1f  %.1f  deg/s"),
        FMath::Abs(AngVel.Y), FMath::Abs(AngVel.Z), FMath::Abs(AngVel.X)),
        CBlue, PX, Y, GEngine->GetSmallFont(), 1.1f); Y += LH*1.3f;

    DrawText(TEXT("FUEL"), bLow ? CYellow : CText, PX, Y, GEngine->GetSmallFont(), 0.9f); Y += LH*0.8f;
    DrawBar(PX, Y, PW, 12.0f, static_cast<float>(FuelFrac), bLow ? CRed : CGreen); Y += 16.0f;
    DrawText(FString::Printf(TEXT("%.1f %%"), FuelFrac*100.0),
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

    const float DX   = SW - 310.0f;
    const float DY   = 28.0f;
    const float DW   = 280.0f;
    const float DH   = 290.0f;
    const float BW   = 44.0f;
    const float BH   = 16.0f;
    const float Gap  = 4.0f;

    DrawRect(CBg, DX-Pad, DY-Pad, DW+Pad*2, DH+Pad*2);
    DrawText(TEXT("THRUSTERS"), CText, DX, DY, GEngine->GetSmallFont(), 1.0f);

    const FThrusterSystem& TS = Pawn->GetThrusterSystem();

    auto Box = [&](float X, float Y, EThrusterID ID, const TCHAR* Label, FLinearColor ActiveColor)
    {
        const bool bOn = TS.IsActive(ID);
        DrawRect(bOn ? ActiveColor : CDim, X, Y, BW, BH);
        DrawText(FString(Label), bOn ? FLinearColor::Black : CText,
                 X+2, Y+2, GEngine->GetSmallFont(), 0.70f);
    };

    // Centre of the diagram
    const float CX = DX + DW * 0.5f;
    const float CY = DY + 50.0f;

    // ── FRONT cluster (top row) ──
    const float FY = CY;
    const float FStartX = CX - (BW*2 + Gap*1.5f);
    DrawText(TEXT("FRONT"), CText, FStartX, FY-14, GEngine->GetSmallFont(), 0.8f);
    Box(FStartX,                FY, EThrusterID::FRONT_UP,    TEXT("UP"),    COn);
    Box(FStartX + BW+Gap,       FY, EThrusterID::FRONT_DOWN,  TEXT("DOWN"),  COn);
    Box(FStartX + (BW+Gap)*2,   FY, EThrusterID::FRONT_LEFT,  TEXT("LEFT"),  COn);
    Box(FStartX + (BW+Gap)*3,   FY, EThrusterID::FRONT_RIGHT, TEXT("RIGHT"), COn);

    // ── MAIN engine (centre) ──
    const float MY = CY + BH + Gap*4;
    const float MX = CX - BW*0.5f;
    DrawRect(TS.IsActive(EThrusterID::MAIN) ? CMain : CDim, MX, MY, BW, BH+4);
    DrawText(TEXT("MAIN"), TS.IsActive(EThrusterID::MAIN) ? FLinearColor::Black : CText,
             MX+4, MY+3, GEngine->GetSmallFont(), 0.85f);
    DrawText(TEXT("[SPACE]"), CText, MX-2, MY+BH+6, GEngine->GetSmallFont(), 0.70f);

    // ── LEFT cluster ──
    const float LX = DX + 2.0f;
    const float LY = MY - BH*0.5f;
    DrawText(TEXT("LEFT"), CText, LX, LY-14, GEngine->GetSmallFont(), 0.8f);
    Box(LX,          LY,           EThrusterID::LEFT_UP,   TEXT("UP"),  COn);
    Box(LX+BW+Gap,   LY,           EThrusterID::LEFT_DOWN, TEXT("DN"),  COn);
    Box(LX,          LY+BH+Gap,    EThrusterID::LEFT_LEFT,  TEXT("LEFT"), COn);
    Box(LX+BW+Gap,   LY+BH+Gap,    EThrusterID::LEFT_RIGHT,  TEXT("RIGHT"), COn);

    // ── RIGHT cluster ──
    const float RX = DX + DW - BW*2 - Gap - 2.0f;
    const float RY = LY;
    DrawText(TEXT("RIGHT"), CText, RX, RY-14, GEngine->GetSmallFont(), 0.8f);
    Box(RX,          RY,           EThrusterID::RIGHT_UP,   TEXT("UP"),  COn);
    Box(RX+BW+Gap,   RY,           EThrusterID::RIGHT_DOWN, TEXT("DN"),  COn);
    Box(RX,          RY+BH+Gap,    EThrusterID::RIGHT_LEFT,  TEXT("LEFT"), COn);
    Box(RX+BW+Gap,   RY+BH+Gap,    EThrusterID::RIGHT_RIGHT,  TEXT("RIGHT"), COn);

    // ── AFT cluster (bottom row) ──
    const float AftY = MY + BH + Gap*5;
    const float AftStartX = FStartX;
    DrawText(TEXT("AFT"), CText, AftStartX, AftY-14, GEngine->GetSmallFont(), 0.8f);
    Box(AftStartX,              AftY, EThrusterID::AFT_UP,    TEXT("UP"),    COn);
    Box(AftStartX + BW+Gap,     AftY, EThrusterID::AFT_DOWN,  TEXT("DOWN"),  COn);
    Box(AftStartX + (BW+Gap)*2, AftY, EThrusterID::AFT_LEFT,  TEXT("LEFT"),  COn);
    Box(AftStartX + (BW+Gap)*3, AftY, EThrusterID::AFT_RIGHT, TEXT("RIGHT"), COn);

    // ── Key reference under diagram ──
    const float KY = DY + DH - 10.0f;
    DrawText(TEXT("1-4: FRONT  5-8: AFT"), CText, DX, KY,    GEngine->GetSmallFont(), 0.85f);
    DrawText(TEXT("Num7/1/4/8: LEFT   Num9/3/6/2: RIGHT"), CText, DX, KY+16, GEngine->GetSmallFont(), 0.85f);

    // ================================================================
    // BOTTOM LEFT – controls
    // ================================================================
    const float HX = 28.0f, HY = SH - 110.0f;
    DrawRect(CBg, HX-Pad, HY-Pad, 240.0f, 100.0f);
    DrawText(TEXT("CONTROLS"),             CText, HX, HY,    GEngine->GetSmallFont(), 1.0f);
    DrawText(TEXT("Space   – main engine"),CText, HX, HY+24, GEngine->GetSmallFont(), 1.0f);
    DrawText(TEXT("1-8     – FRONT/AFT clusters"), CText, HX, HY+46, GEngine->GetSmallFont(), 1.0f);
    DrawText(TEXT("Numpad  – LEFT/RIGHT clusters"),CText, HX, HY+68, GEngine->GetSmallFont(), 1.0f);
}

void ASpacecraftHUD::DrawBar(float X, float Y, float W, float H,
                              float Fraction, FLinearColor Fill)
{
    DrawRect(FLinearColor(0.15f, 0.15f, 0.15f, 0.8f), X, Y, W, H);
    if (Fraction > 0.0f)
        DrawRect(Fill, X, Y, W * FMath::Clamp(Fraction, 0.0f, 1.0f), H);
}
