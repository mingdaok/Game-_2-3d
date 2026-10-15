#include "FoldlightDemo.h"
#include "FDVisuals.h"
#include "FDProjection.h"
#include "Engine/Canvas.h"
#include "CanvasItem.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/FontFace.h"
#include "Kismet/GameplayStatics.h"

void AFDHUD::Text(const FString& Value,float X,float Y,float Size,FLinearColor Color,bool Centered)
{
    if (!ChineseFont)
    {
        UFontFace* Face=LoadObject<UFontFace>(nullptr,TEXT("/Game/Foldlight/Fonts/DroidSansFallback.DroidSansFallback"));
        checkf(Face,TEXT("The packaged Chinese font face is required by Foldlight HUD."));
        ChineseFont=NewObject<UFont>(this);
        ChineseFont->FontCacheType=EFontCacheType::Runtime;
        ChineseFont->LegacyFontSize=10;
        FTypefaceEntry Entry(FName(TEXT("Regular")));
        Entry.Font=FFontData(Face);
        ChineseFont->GetMutableInternalCompositeFont().DefaultTypeface.Fonts.Add(Entry);
    }
    const float S=Canvas->ClipY/720.f;
    // Rasterize at the final point size instead of enlarging a small glyph atlas.
    const FSlateFontInfo FontInfo(ChineseFont,15.f*Size*S,FName(TEXT("Regular")));
    FCanvasTextItem Item(FVector2D(X*S,Y*S),FText::FromString(Value),FontInfo,Color);
    Item.bCentreX=Centered;
    Item.EnableShadow(FLinearColor(0,0,0,.75f),FVector2D(S,2*S));
    Canvas->DrawItem(Item);
}
void AFDHUD::Panel(float X,float Y,float W,float H,float Alpha)
{
    const float S=Canvas->ClipY/720.f;
    DrawRect(FLinearColor(.012f,.023f,.03f,Alpha),X*S,Y*S,W*S,H*S);
}
void AFDHUD::DrawHUD()
{
    Super::DrawHUD();
    AFDWorld* D=Cast<AFDWorld>(UGameplayStatics::GetActorOfClass(this,AFDWorld::StaticClass()));
    if (!D || !D->Player || !Canvas) return;
    const float W=Canvas->ClipX/(Canvas->ClipY/720.f);
    const FLinearColor White(.94f,.91f,.82f), Muted(.54f,.66f,.68f), Gold=FDVisuals::Gold, Teal=FDVisuals::Edge;
    if (!D->bStarted)
    {
        Panel(0,0,W,720,.27f); Panel(0,0,650,720,.83f);
        Text(TEXT("一束微光，照见折叠的世界。"),55,75,.8f,Teal);
        Text(TEXT("折光遗迹"),50,155,3.9f,White);
        Text(TEXT("四向折叠试玩 · 五重试炼"),56,245,1.05f,Gold);
        Text(TEXT("在三维世界中，重排前行的道路。"),56,330,1.05f,White);
        Text(TEXT("旋转四个视角，让远近化为脚下的阶梯。"),56,366,1.05f,White);
        Text(TEXT("将一圈火印，化作一面烈焰之墙。"),56,402,1.05f,White);
        Text(D->Save && D->Save->Room>0 && !D->Save->Finished ? TEXT("回车键   /   继续旅程") : TEXT("回车键   /   开始旅程"),56,510,1.15f,Gold);
        Text(TEXT("N  新的旅程     F10  退出游戏"),56,558,.9f,Muted);
        Text(TEXT("试玩版 0.5   /   使用虚幻引擎 5.7 制作"),56,662,.68f,Muted);
        return;
    }
    if (D->bFinished)
    {
        Panel(0,0,W,720,.85f);
        Text(TEXT("世界重新找回了它的纵深。"),65,125,.95f,Teal);
        Text(TEXT("微光重燃"),60,205,3.1f,White);
        Text(TEXT("五重试炼，一簇微火，照亮前路。"),65,302,1.1f,White);
        Text(FString::Printf(TEXT("用时 %02d:%02d   /   折叠 %d 次   /   坠落 %d 次"),int(D->SessionTime)/60,int(D->SessionTime)%60,D->Switches,D->Falls),65,379,1,Gold);
        Text(TEXT("你已完成第一版试玩的全部关卡。"),65,453,.95f,Muted);
        Text(TEXT("回车键  再玩一次       F10  退出游戏"),65,560,1.1f,Gold);
        return;
    }
    Panel(25,25,615,95,.78f);
    Text(FString::Printf(TEXT("0%d  /  %s"),D->CurrentRoom+1,*D->RoomTitle()),43,39,1.12f,White);
    Text(D->Objective(),43,79,.73f,Muted);
    Panel(W-250,25,225,125,.86f);
    Text(D->Player->b2D ? TEXT("二维 / 已折叠") : TEXT("三维 / 已展开"),W-193,39,1.05f,D->Player->b2D ? Gold : Teal);
    const FString ViewHelp=D->Player->b2D ? FString::Printf(TEXT("%s · 先用 V 展开，再换方向"),FDProjection::Name(D->Player->FoldView)) : FString::Printf(TEXT("%s · Q / F 换方向"),FDProjection::Name(D->Player->FoldView));
    Text(ViewHelp,W-233,110,.62f,Gold);
    for (int I=0;I<5;++I)
    {
        float S=Canvas->ClipY/720.f;
        DrawRect(I<=D->CurrentRoom ? Gold : FLinearColor(.12f,.19f,.20f), (W-191+I*31)*S,86*S,22*S,4*S);
    }
    if (D->CurrentRoom>=2)
    {
        Panel(25,139,215,69,.73f);
        Text(D->Player->b2D ? TEXT("G  /  烈焰之墙") : TEXT("G  /  火焰印记"),43,151,.9f,Gold);
        Text(D->FireRemaining>0 ? FString::Printf(TEXT("剩余 %.1f 秒 / 一枚火印"),D->FireRemaining) : TEXT("已就绪 / 替换现有火印"),43,181,.67f,Muted);
    }
    if (D->CurrentRoom==4)
    {
        Panel(25,222,215,110,.85f);
        Text(TEXT("四向印记 · 南 西 北 东"),43,234,.74f,White);
        for (int32 I=0;I<4;++I)
        {
            const int32 Order[]={0,3,2,1};
            const int32 Face=Order[I];
            Text(FString::Printf(TEXT("%s %s"),FDProjection::Name(Face),D->Find(FString::Printf(TEXT("r4_seal%d"),Face))->bActivated ? TEXT("已亮") : TEXT("未亮")),43+(I%2)*96,264+(I/2)*27,.69f,D->Find(FString::Printf(TEXT("r4_seal%d"),Face))->bActivated ? Gold : Muted);
        }
    }
    const FString Action=D->ContextAction();
    if (!Action.IsEmpty()) { Panel(W*.5f-220,548,440,43,.84f); Text(Action,W*.5f-202,558,.95f,Gold); }
    if (D->ToastRemaining>0 && !D->Player->bPreview)
    {
        Panel(25,603,W-50,45,.86f); Text(D->Toast,43,616,.78f,White);
    }
    Panel(0,671,W,49,.91f);
    Text(TEXT("WASD 移动 / 空格 跳跃 / Q·F 选方向 / 按住 V 预览，松开切换 / G 施法 / E 交互 / R 重置 / H 提示 / Esc 暂停"),25,685,.72f,Muted);
    if (D->Player->bPreview)
    {
        FVector Destination; FString Reason;
        const bool Valid=D->Player->CanSwitch(Destination,Reason);
        Panel(25,592,W-50,66,.95f);
        Text(Valid ? TEXT("松开 V，切换维度") : TEXT("暂时无法切换维度"),43,602,.95f,Valid ? Teal : Gold);
        Text(Reason,43,633,.72f,White);
        if (Valid && D->Player->b2D)
        {
            FVector2D P;
            if (PlayerOwner->ProjectWorldLocationToScreen(Destination,P))
            {
                DrawLine(P.X-18,P.Y,P.X+18,P.Y,Gold,2);
                DrawLine(P.X,P.Y-18,P.X,P.Y+18,Gold,2);
            }
        }
    }
    if (D->bPaused)
    {
        Panel(0,0,W,720,.82f);
        Text(TEXT("稍作歇息"),W*.5f,230,2.3f,White,true);
        Text(TEXT("Esc / 回车键  继续游戏"),W*.5f,365,1.1f,Gold,true);
        Text(TEXT("F10  退出到桌面"),W*.5f,415,.9f,Muted,true);
        Text(TEXT("每通过一个房间，进度都会自动保存。"),W*.5f,500,.85f,Muted,true);
    }
}
