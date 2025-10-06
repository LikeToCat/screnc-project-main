#pragma once
#include <SDL.h>
#include <vector>
#include <chrono>
#include <random>
#include <memory>
#include <functional>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 缓动函数枚举
enum class EasingType
{
    Linear,
    EaseInQuad, EaseOutQuad, EaseInOutQuad,
    EaseInCubic, EaseOutCubic, EaseInOutCubic,
    EaseInElastic, EaseOutElastic, EaseInOutElastic,
    EaseInBounce, EaseOutBounce, EaseInOutBounce
};

// 粒子结构
struct Particle
{
    float x, y;           // 位置
    float vx, vy;         // 速度
    float life;           // 生命值 (0.0 - 1.0)
    float maxLife;        // 最大生命值
    float size;           // 大小
    float rotation;       // 旋转角度
    float rotationSpeed;  // 旋转速度
    SDL_Color color;      // 颜色
    bool active;          // 是否活跃

    // 物理属性
    float gravity;        // 重力影响
    float friction;       // 摩擦力
    float bounce;         // 弹性系数
};

// 光线结构体
struct LightRay
{
    float angle;          // 角度
    float length;         // 长度
    float intensity;      // 强度
    float thickness;      // 粗细
    float pulsSpeed;      // 脉冲速度
    float baseIntensity;  // 基础强度
    SDL_Color color;      // 颜色
    bool active;          // 是否激活

    // 动画属性
    float animOffset;     // 动画偏移
    float rotationSpeed;  // 旋转速度
};

// 光芒粒子
struct GlowParticle
{
    float x, y;           // 位置
    float angle;          // 角度
    float distance;       // 距离中心的距离
    float size;           // 大小
    float alpha;          // 透明度
    float speed;          // 移动速度
    float life;           // 生命值
    float maxLife;        // 最大生命值
    SDL_Color color;      // 颜色
    bool active;          // 是否激活
};

// 缓动动画类
class TweenAnimation
{
public:
    TweenAnimation(float startValue, float endValue, float duration, EasingType easing = EasingType::EaseOutQuad);

    void Update(float deltaTime);
    float GetCurrentValue() const { return m_currentValue; }
    bool IsComplete() const { return m_isComplete; }
    void Reset();
    void SetOnComplete(std::function<void()> callback) { m_onComplete = callback; }

private:
    float ApplyEasing(float t, EasingType type) const;

private:
    float m_startValue;
    float m_endValue;
    float m_duration;
    float m_currentTime;
    float m_currentValue;
    EasingType m_easingType;
    bool m_isComplete;
    std::function<void()> m_onComplete;
};

// 粒子系统类
class ParticleSystem
{
public:
    ParticleSystem(int maxParticles = 100);
    ~ParticleSystem() = default;

    void Update(float deltaTime, const SDL_Rect& buttonRect);
    void Render(SDL_Renderer* renderer);

    // 发射粒子
    void EmitGoldSparkles(int count, float centerX, float centerY, float radius = 50.0f);
    void EmitRippleEffect(float centerX, float centerY, float intensity = 1.0f);
    void EmitMagicTrail(float x, float y, const SDL_Color& color);
    void EmitClickExplosion(float centerX, float centerY);

    void Clear();

private:
    std::vector<Particle> m_particles;
    std::random_device m_rd;
    std::mt19937 m_gen;
    std::uniform_real_distribution<float> m_dis;
    int m_maxParticles;

    void UpdateParticle(Particle& particle, float deltaTime, const SDL_Rect& buttonRect);
    void CreateParticle(float x, float y, float vx, float vy, const SDL_Color& color, float life, float size = 2.0f);
};

// 光芒四射动画类
class RadiantGlowAnimator
{
public:
    RadiantGlowAnimator();
    ~RadiantGlowAnimator() = default;

    void Initialize();
    void Update(float deltaTime, const SDL_Rect& buttonRect, bool isHovered, bool isClicked);
    void Render(SDL_Renderer* renderer, const SDL_Rect& buttonRect);

    void SetIntensity(float intensity) { m_globalIntensity = intensity; }
    void SetRotationSpeed(float speed) { m_rotationSpeed = speed; }
    void SetGlowColor(const SDL_Color& color) { m_glowColor = color; }

private:
    void InitializeLightRays();
    void InitializeGlowParticles();

    void UpdateLightRays(float deltaTime);
    void UpdateGlowParticles(float deltaTime, const SDL_Rect& buttonRect);

    void RenderLightRays(SDL_Renderer* renderer, const SDL_Rect& buttonRect);
    void RenderGlowParticles(SDL_Renderer* renderer, const SDL_Rect& buttonRect);
    void RenderCenterGlow(SDL_Renderer* renderer, const SDL_Rect& buttonRect);
    void RenderRotatingHalo(SDL_Renderer* renderer, const SDL_Rect& buttonRect);
    void RenderEnergyWaves(SDL_Renderer* renderer, const SDL_Rect& buttonRect);

    // 工具函数
    void DrawGradientLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2,
        const SDL_Color& startColor, const SDL_Color& endColor, float thickness);
    void DrawGlowCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius,
        const SDL_Color& color, float intensity);
    SDL_Color BlendColors(const SDL_Color& color1, const SDL_Color& color2, float t);

private:
    std::vector<LightRay> m_lightRays;
    std::vector<GlowParticle> m_glowParticles;

    float m_totalTime;
    float m_rotationAngle;
    float m_rotationSpeed;
    float m_globalIntensity;
    float m_pulsePhase;

    SDL_Color m_glowColor;

    // 随机数生成
    std::random_device m_rd;
    std::mt19937 m_gen;
    std::uniform_real_distribution<float> m_dis;

    // 多层光环效果
    struct HaloRing
    {
        float radius;
        float rotationSpeed;
        float alpha;
        int segmentCount;
        SDL_Color color;
    };
    std::vector<HaloRing> m_haloRings;

    // 能量波动效果
    struct EnergyWave
    {
        float radius;
        float maxRadius;
        float alpha;
        float speed;
        bool active;
    };
    std::vector<EnergyWave> m_energyWaves;
    float m_waveSpawnTimer;
};

// 高级按钮动画管理器
class AdvancedButtonAnimator
{
public:
    AdvancedButtonAnimator();
    ~AdvancedButtonAnimator() = default;

    void Initialize(SDL_Renderer* renderer);
    void Update(float deltaTime, const SDL_Rect& buttonRect, bool isHovered, bool isClicked);
    void Render(SDL_Renderer* renderer, const SDL_Rect& buttonRect);

    void TriggerClickEffect(const SDL_Rect& buttonRect);
    void TriggerHoverEffect(const SDL_Rect& buttonRect);

private:
    void RenderGlowEffect(SDL_Renderer* renderer, const SDL_Rect& buttonRect);
    void RenderShineEffect(SDL_Renderer* renderer, const SDL_Rect& buttonRect);
    void RenderPulseRings(SDL_Renderer* renderer, const SDL_Rect& buttonRect);
    void RenderMagicAura(SDL_Renderer* renderer, const SDL_Rect& buttonRect);
    void RenderLightningEffect(SDL_Renderer* renderer, const SDL_Rect& buttonRect);

    SDL_Color InterpolateColor(const SDL_Color& start, const SDL_Color& end, float t);
    void DrawGradientCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius, const SDL_Color& innerColor, const SDL_Color& outerColor);

private:
    SDL_Renderer* m_renderer;
    ParticleSystem m_particleSystem;

    // 动画状态
    std::unique_ptr<TweenAnimation> m_glowIntensity;
    std::unique_ptr<TweenAnimation> m_pulseScale;
    std::unique_ptr<TweenAnimation> m_shinePosition;
    std::unique_ptr<TweenAnimation> m_rotationAngle;
    std::unique_ptr<TweenAnimation> m_auraRadius;

    // 光芒四射动画
    std::unique_ptr<RadiantGlowAnimator> m_radiantGlow; 

    // 时间和随机
    float m_totalTime;
    std::random_device m_rd;
    std::mt19937 m_gen;

    // 效果状态
    bool m_isHovered;
    bool m_wasHovered;
    bool m_isClicked;
    float m_clickEffectTime;

    // 魔法光环效果
    struct AuraRing
    {
        float radius;
        float alpha;
        float rotationSpeed;
        SDL_Color color;
    };
    std::vector<AuraRing> m_auraRings;
};