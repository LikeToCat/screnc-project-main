#include "stdafx.h"
#include "SDL_AnimaSystemCommon.h"
#include <algorithm>
#include <cmath>

// =============== TweenAnimation ʵ�� ===============

TweenAnimation::TweenAnimation(float startValue, float endValue, float duration, EasingType easing)
    : m_startValue(startValue), m_endValue(endValue), m_duration(duration),
    m_currentTime(0.0f), m_currentValue(startValue), m_easingType(easing), m_isComplete(false)
{
}

void TweenAnimation::Update(float deltaTime)
{
    if (m_isComplete) return;

    m_currentTime += deltaTime;
    if (m_currentTime >= m_duration)
    {
        m_currentTime = m_duration;
        m_isComplete = true;
    }

    // Compute final value first
    float t = (m_duration > 0.0f) ? (m_currentTime / m_duration) : 1.0f;
    t = ApplyEasing(t, m_easingType);
    m_currentValue = m_startValue + (m_endValue - m_startValue) * t;

    // Then invoke completion callback (copy to avoid re-entrancy issues)
    if (m_isComplete && m_onComplete)
    {
        auto cb = m_onComplete;
        cb();
    }
}

void TweenAnimation::Reset()
{
    m_currentTime = 0.0f;
    m_currentValue = m_startValue;
    m_isComplete = false;
}

float TweenAnimation::ApplyEasing(float t, EasingType type) const
{
    switch (type)
    {
    case EasingType::Linear:
        return t;

    case EasingType::EaseOutQuad:
        return 1.0f - (1.0f - t) * (1.0f - t);

    case EasingType::EaseInOutQuad:
        return t < 0.5f ? 2.0f * t * t : 1.0f - pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;

    case EasingType::EaseOutElastic:
    {
        const float c4 = (2.0f * M_PI) / 3.0f;
        return t == 0.0f ? 0.0f : t == 1.0f ? 1.0f : pow(2.0f, -10.0f * t) * sin((t * 10.0f - 0.75f) * c4) + 1.0f;
    }

    case EasingType::EaseOutBounce:
    {
        const float n1 = 7.5625f;
        const float d1 = 2.75f;

        if (t < 1.0f / d1) {
            return n1 * t * t;
        }
        else if (t < 2.0f / d1) {
            return n1 * (t -= 1.5f / d1) * t + 0.75f;
        }
        else if (t < 2.5f / d1) {
            return n1 * (t -= 2.25f / d1) * t + 0.9375f;
        }
        else {
            return n1 * (t -= 2.625f / d1) * t + 0.984375f;
        }
    }

    default:
        return t;
    }
}

// =============== ParticleSystem ʵ�� ===============

ParticleSystem::ParticleSystem(int maxParticles)
    : m_maxParticles(maxParticles), m_gen(m_rd()), m_dis(0.0f, 1.0f)
{
    m_particles.reserve(maxParticles);
}

void ParticleSystem::Update(float deltaTime, const SDL_Rect& buttonRect)
{
    for (auto& particle : m_particles)
    {
        if (particle.active)
        {
            UpdateParticle(particle, deltaTime, buttonRect);
        }
    }

    m_particles.erase(
        std::remove_if(m_particles.begin(), m_particles.end(),
            [](const Particle& p) { return !p.active; }),
        m_particles.end());
}

void ParticleSystem::UpdateParticle(Particle& particle, float deltaTime, const SDL_Rect& buttonRect)
{
    particle.life -= deltaTime;
    if (particle.life <= 0.0f)
    {
        particle.active = false;
        return;
    }

    particle.x += particle.vx * deltaTime;
    particle.y += particle.vy * deltaTime;

    particle.vy += particle.gravity * deltaTime;

    particle.vx *= (1.0f - particle.friction * deltaTime);
    particle.vy *= (1.0f - particle.friction * deltaTime);

    particle.rotation += particle.rotationSpeed * deltaTime;

    float lifeRatio = particle.life / particle.maxLife;
    particle.color.a = (Uint8)(255.0f * lifeRatio);
}

void ParticleSystem::EmitGoldSparkles(int count, float centerX, float centerY, float radius)
{
    for (int i = 0; i < count; ++i)
    {
        float angle = m_dis(m_gen) * 2.0f * M_PI;
        float distance = m_dis(m_gen) * radius;
        float speed = 20.0f + m_dis(m_gen) * 40.0f;

        float x = centerX + cos(angle) * distance;
        float y = centerY + sin(angle) * distance;
        float vx = cos(angle) * speed;
        float vy = sin(angle) * speed - 20.0f; // ���ϵĳ�ʼ�ٶ�

        SDL_Color color = { 255, 215 + (Uint8)(m_dis(m_gen) * 40), 100 + (Uint8)(m_dis(m_gen) * 50), 255 };
        float life = 1.0f + m_dis(m_gen) * 2.0f;
        float size = 1.0f + m_dis(m_gen) * 3.0f;

        CreateParticle(x, y, vx, vy, color, life, size);
    }
}

void ParticleSystem::EmitClickExplosion(float centerX, float centerY)
{
    for (int i = 0; i < 30; ++i)
    {
        float angle = (float)i / 30.0f * 2.0f * M_PI;
        float speed = 80.0f + m_dis(m_gen) * 60.0f;

        float vx = cos(angle) * speed;
        float vy = sin(angle) * speed;

        SDL_Color color = { 255, 200 + (Uint8)(m_dis(m_gen) * 55), 80 + (Uint8)(m_dis(m_gen) * 80), 255 };
        CreateParticle(centerX, centerY, vx, vy, color, 0.8f, 2.5f);
    }

    EmitGoldSparkles(20, centerX, centerY, 10.0f);
}

void ParticleSystem::CreateParticle(float x, float y, float vx, float vy, const SDL_Color& color, float life, float size)
{
    if (m_particles.size() >= m_maxParticles) return;

    Particle particle;
    particle.x = x;
    particle.y = y;
    particle.vx = vx;
    particle.vy = vy;
    particle.life = life;
    particle.maxLife = life;
    particle.size = size;
    particle.rotation = m_dis(m_gen) * 360.0f;
    particle.rotationSpeed = (m_dis(m_gen) - 0.5f) * 360.0f;
    particle.color = color;
    particle.active = true;
    particle.gravity = 50.0f;
    particle.friction = 0.5f;
    particle.bounce = 0.3f;

    m_particles.push_back(particle);
}

void ParticleSystem::Render(SDL_Renderer* renderer)
{
    for (const auto& particle : m_particles)
    {
        if (!particle.active) continue;

        SDL_SetRenderDrawColor(renderer, particle.color.r, particle.color.g, particle.color.b, particle.color.a);

        SDL_Rect particleRect = {
            (int)(particle.x - particle.size / 2),
            (int)(particle.y - particle.size / 2),
            (int)particle.size,
            (int)particle.size
        };

        SDL_RenderFillRect(renderer, &particleRect);

        if (particle.size > 2.0f)
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 200, particle.color.a / 4);
            SDL_Rect glowRect = {
                (int)(particle.x - particle.size),
                (int)(particle.y - particle.size),
                (int)(particle.size * 2),
                (int)(particle.size * 2)
            };
            SDL_RenderFillRect(renderer, &glowRect);
        }
    }
}

void ParticleSystem::Clear()
{
    m_particles.clear();
}

// =============== AdvancedButtonAnimator ʵ�� ===============

AdvancedButtonAnimator::AdvancedButtonAnimator()
    : m_renderer(nullptr), m_totalTime(0.0f), m_gen(m_rd()),
    m_isHovered(false), m_wasHovered(false), m_isClicked(false), m_clickEffectTime(0.0f)
{
    m_auraRings.resize(3);
    for (int i = 0; i < 3; ++i)
    {
        m_auraRings[i].radius = 30.0f + i * 15.0f;
        m_auraRings[i].alpha = 0.3f - i * 0.1f;
        m_auraRings[i].rotationSpeed = 30.0f + i * 20.0f;
        Uint8 r = 255;
        Uint8 g = 215 - i * 30;
        Uint8 b = 100 + i * 20;
        Uint8 a = 100 - i * 20;
        m_auraRings[i].color = { r, g, b, a };
    }
}

void AdvancedButtonAnimator::Initialize(SDL_Renderer* renderer)
{
    m_renderer = renderer;
    m_radiantGlow = std::make_unique<RadiantGlowAnimator>();
    m_radiantGlow->Initialize();
    m_radiantGlow->SetGlowColor({ 255, 215, 100, 255 }); // ��ɫ

    m_glowIntensity = std::make_unique<TweenAnimation>(0.0f, 1.0f, 2.0f, EasingType::EaseInOutQuad);
    m_pulseScale = std::make_unique<TweenAnimation>(1.0f, 1.1f, 1.5f, EasingType::EaseOutElastic);
    m_shinePosition = std::make_unique<TweenAnimation>(-100.0f, 300.0f, 3.0f, EasingType::Linear);
    m_rotationAngle = std::make_unique<TweenAnimation>(0.0f, 360.0f, 8.0f, EasingType::Linear);
    m_auraRadius = std::make_unique<TweenAnimation>(20.0f, 60.0f, 2.5f, EasingType::EaseOutQuad);

    // 循环旋转
    m_rotationAngle->SetOnComplete([this]() {
        m_rotationAngle->Reset();
        });
}

void AdvancedButtonAnimator::Update(float deltaTime, const SDL_Rect& buttonRect, bool isHovered, bool isClicked)
{
    m_totalTime += deltaTime;

    m_wasHovered = m_isHovered;
    m_isHovered = isHovered;
    m_isClicked = isClicked;

    if (m_glowIntensity) m_glowIntensity->Update(deltaTime);
    if (m_pulseScale) m_pulseScale->Update(deltaTime);
    if (m_shinePosition) m_shinePosition->Update(deltaTime);
    if (m_rotationAngle) m_rotationAngle->Update(deltaTime);
    if (m_auraRadius) m_auraRadius->Update(deltaTime);

    // 在 Update 之后检查完成并重建，不在回调里重建
    if (m_glowIntensity && m_glowIntensity->IsComplete())
    {
        float newStart = m_glowIntensity->GetCurrentValue();
        float newEnd = (newStart > 0.5f) ? 0.2f : 1.0f;
        m_glowIntensity = std::make_unique<TweenAnimation>(
            newStart,
            newEnd,
            2.0f + (rand() % 100) * 0.01f,
            EasingType::EaseInOutQuad);
    }

    if (m_isHovered && !m_wasHovered)
    {
        TriggerHoverEffect(buttonRect);
    }

    if (m_clickEffectTime > 0.0f)
    {
        m_clickEffectTime -= deltaTime;
    }

    m_particleSystem.Update(deltaTime, buttonRect);

    if (m_isHovered && ((int)(m_totalTime * 10) % 3 == 0))
    {
        float centerX = buttonRect.x + buttonRect.w / 2.0f;
        float centerY = buttonRect.y + buttonRect.h / 2.0f;
        m_particleSystem.EmitGoldSparkles(2, centerX, centerY, 40.0f);
    }

    if (m_shinePosition && m_shinePosition->IsComplete())
    {
        m_shinePosition->Reset();
    }

    if (m_radiantGlow)
    {
        m_radiantGlow->Update(deltaTime, buttonRect, isHovered, isClicked);
    }
}

void AdvancedButtonAnimator::Render(SDL_Renderer* renderer, const SDL_Rect& buttonRect)
{
    if (m_radiantGlow)
    {
        m_radiantGlow->Render(renderer, buttonRect);
    }

    RenderMagicAura(renderer, buttonRect);

    RenderGlowEffect(renderer, buttonRect);

    RenderPulseRings(renderer, buttonRect);

    RenderShineEffect(renderer, buttonRect);

    m_particleSystem.Render(renderer);

    if (m_clickEffectTime > 0.0f)
    {
        RenderLightningEffect(renderer, buttonRect);
    }
}

void AdvancedButtonAnimator::RenderGlowEffect(SDL_Renderer* renderer, const SDL_Rect& buttonRect)
{
    if (!m_glowIntensity) return;

    float intensity = m_glowIntensity->GetCurrentValue();
    if (m_isHovered) intensity *= 1.5f;

    int glowRadius = (int)(15.0f * intensity);
    SDL_Color innerColor = { 255, 235, 150, (Uint8)(60 * intensity) };
    SDL_Color outerColor = { 255, 200, 80, (Uint8)(20 * intensity) };

    int centerX = buttonRect.x + buttonRect.w / 2;
    int centerY = buttonRect.y + buttonRect.h / 2;

    for (int i = 0; i < 5; ++i)
    {
        int radius = glowRadius + i * 3;
        Uint8 alpha = (Uint8)(innerColor.a * (5 - i) / 5.0f);

        for (int angle = 0; angle < 360; angle += 10)
        {
            float rad = angle * M_PI / 180.0f;
            int x = centerX + (int)(cos(rad) * radius);
            int y = centerY + (int)(sin(rad) * radius);

            SDL_SetRenderDrawColor(renderer, innerColor.r, innerColor.g, innerColor.b, alpha);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

void AdvancedButtonAnimator::RenderShineEffect(SDL_Renderer* renderer, const SDL_Rect& buttonRect)
{
    if (!m_shinePosition) return;

    float shineX = m_shinePosition->GetCurrentValue();

    for (int i = -2; i <= 2; ++i)
    {
        int x = (int)(buttonRect.x + shineX + i * 3);
        if (x < buttonRect.x || x > buttonRect.x + buttonRect.w) continue;

        Uint8 alpha = (Uint8)(120 - abs(i) * 30);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
        SDL_RenderDrawLine(renderer, x, buttonRect.y, x, buttonRect.y + buttonRect.h);
    }
}

void AdvancedButtonAnimator::RenderPulseRings(SDL_Renderer* renderer, const SDL_Rect& buttonRect)
{
    if (!m_pulseScale) return;

    float scale = m_pulseScale->GetCurrentValue();
    int centerX = buttonRect.x + buttonRect.w / 2;
    int centerY = buttonRect.y + buttonRect.h / 2;

    for (int ring = 0; ring < 3; ++ring)
    {
        int radius = (int)(20.0f * scale + ring * 8);
        Uint8 alpha = (Uint8)(80 - ring * 25);

        SDL_SetRenderDrawColor(renderer, 255, 215, 100, alpha);

        for (int angle = 0; angle < 360; angle += 5)
        {
            float rad = angle * M_PI / 180.0f;
            int x = centerX + (int)(cos(rad) * radius);
            int y = centerY + (int)(sin(rad) * radius);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

void AdvancedButtonAnimator::RenderMagicAura(SDL_Renderer* renderer, const SDL_Rect& buttonRect)
{
    int centerX = buttonRect.x + buttonRect.w / 2;
    int centerY = buttonRect.y + buttonRect.h / 2;

    for (size_t i = 0; i < m_auraRings.size(); ++i)
    {
        const auto& ring = m_auraRings[i];
        float rotation = m_totalTime * ring.rotationSpeed;

        for (int symbol = 0; symbol < 8; ++symbol)
        {
            float angle = (symbol * 45.0f + rotation) * M_PI / 180.0f;
            int x = centerX + (int)(cos(angle) * ring.radius);
            int y = centerY + (int)(sin(angle) * ring.radius);

            SDL_SetRenderDrawColor(renderer, ring.color.r, ring.color.g, ring.color.b, ring.color.a);

            SDL_Rect symbolRect = { x - 2, y - 2, 4, 4 };
            SDL_RenderFillRect(renderer, &symbolRect);
        }
    }
}

void AdvancedButtonAnimator::RenderLightningEffect(SDL_Renderer* renderer, const SDL_Rect& buttonRect)
{
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);

    int centerX = buttonRect.x + buttonRect.w / 2;
    int centerY = buttonRect.y + buttonRect.h / 2;

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);

    for (int i = 0; i < 5; ++i)
    {
        float angle = dis(m_gen) * M_PI;
        int length = 20 + (int)(dis(m_gen) * 15);

        int endX = centerX + (int)(cos(angle) * length);
        int endY = centerY + (int)(sin(angle) * length);

        SDL_RenderDrawLine(renderer, centerX, centerY, endX, endY);
    }
}

void AdvancedButtonAnimator::TriggerClickEffect(const SDL_Rect& buttonRect)
{
    m_clickEffectTime = 0.5f;

    float centerX = buttonRect.x + buttonRect.w / 2.0f;
    float centerY = buttonRect.y + buttonRect.h / 2.0f;

    m_particleSystem.EmitClickExplosion(centerX, centerY);

    if (m_pulseScale)
    {
        m_pulseScale = std::make_unique<TweenAnimation>(1.0f, 1.2f, 0.3f, EasingType::EaseOutBounce);
    }
}

void AdvancedButtonAnimator::TriggerHoverEffect(const SDL_Rect& buttonRect)
{
    float centerX = buttonRect.x + buttonRect.w / 2.0f;
    float centerY = buttonRect.y + buttonRect.h / 2.0f;

    m_particleSystem.EmitGoldSparkles(10, centerX, centerY, 30.0f);
}

SDL_Color AdvancedButtonAnimator::InterpolateColor(const SDL_Color& start, const SDL_Color& end, float t)
{
    return {
        (Uint8)(start.r + (end.r - start.r) * t),
        (Uint8)(start.g + (end.g - start.g) * t),
        (Uint8)(start.b + (end.b - start.b) * t),
        (Uint8)(start.a + (end.a - start.a) * t)
    };
}

RadiantGlowAnimator::RadiantGlowAnimator()
    : m_totalTime(0.0f), m_rotationAngle(0.0f), m_rotationSpeed(25.0f), 
    m_globalIntensity(1.0f), m_pulsePhase(0.0f), m_gen(m_rd()), m_dis(0.0f, 1.0f),
    m_waveSpawnTimer(0.0f)
{
    m_glowColor = { 255, 215, 100, 255 }; 
}

void RadiantGlowAnimator::Initialize()
{
    InitializeLightRays();
    InitializeGlowParticles();

    m_haloRings.clear();
    m_haloRings.resize(4);

    for (int i = 0; i < 4; ++i)
    {
        m_haloRings[i].radius = 35.0f + i * 12.0f;
        m_haloRings[i].rotationSpeed = 20.0f + i * 15.0f; 
        m_haloRings[i].alpha = 0.4f - i * 0.08f;
        m_haloRings[i].segmentCount = 8 + i * 4;

        m_haloRings[i].color = {
            static_cast<Uint8>(255 - i * 10),
            static_cast<Uint8>(215 - i * 15),
            static_cast<Uint8>(100 + i * 20),
            static_cast<Uint8>(100 - i * 15)
        };
    }
    m_energyWaves.resize(3);
    for (auto& wave : m_energyWaves)
    {
        wave.active = false;
        wave.radius = 0.0f;
        wave.maxRadius = 80.0f;
        wave.speed = 60.0f;
        wave.alpha = 0.6f;
    }
}

void RadiantGlowAnimator::InitializeLightRays()
{
    const int rayCount = 8; // 16������
    m_lightRays.clear();
    m_lightRays.resize(rayCount);

    for (int i = 0; i < rayCount; ++i)
    {
        LightRay& ray = m_lightRays[i];
        ray.angle = (360.0f / rayCount) * i; 
        ray.length = 40.0f + m_dis(m_gen) * 25.0f; 
        ray.intensity = 0.6f + m_dis(m_gen) * 0.4f;
        ray.thickness = 1.5f + m_dis(m_gen) * 1.0f;
        ray.pulsSpeed = 2.0f + m_dis(m_gen) * 3.0f; 
        ray.baseIntensity = ray.intensity;
        ray.active = true;
        ray.animOffset = m_dis(m_gen) * 2.0f * M_PI; 
        ray.rotationSpeed = 1.0f; 

        ray.color = {
            static_cast<Uint8>(255),
            static_cast<Uint8>(215 + m_dis(m_gen) * 40),
            static_cast<Uint8>(100 + m_dis(m_gen) * 50),
            255
        };
    }
}

void RadiantGlowAnimator::InitializeGlowParticles()
{
    const int particleCount = 24; 
    m_glowParticles.clear();
    m_glowParticles.resize(particleCount);

    for (int i = 0; i < particleCount; ++i)
    {
        GlowParticle& particle = m_glowParticles[i];
        particle.angle = m_dis(m_gen) * 360.0f;
        particle.distance = 20.0f + m_dis(m_gen) * 25.0f;
        particle.size = 2.0f + m_dis(m_gen) * 3.0f; 
        particle.alpha = 0.3f + m_dis(m_gen) * 0.5f;
        particle.speed = 15.0f + m_dis(m_gen) * 25.0f; 
        particle.life = 1.0f;
        particle.maxLife = 1.0f;
        particle.active = true;

        particle.color = {
            255,
            static_cast<Uint8>(215 + m_dis(m_gen) * 30),
            static_cast<Uint8>(120 + m_dis(m_gen) * 40),
            static_cast<Uint8>(particle.alpha * 255)
        };
    }
}

void RadiantGlowAnimator::Update(float deltaTime, const SDL_Rect& buttonRect, bool isHovered, bool isClicked)
{
    m_totalTime += deltaTime;

    m_rotationAngle += m_rotationSpeed * deltaTime;
    if (m_rotationAngle >= 360.0f)
        m_rotationAngle -= 360.0f;

    m_pulsePhase += deltaTime * 3.0f; 

    float targetIntensity = 1.0f;
    if (isHovered)
        targetIntensity = 1.5f;
    if (isClicked)
        targetIntensity = 2.0f;

    m_globalIntensity += (targetIntensity - m_globalIntensity) * deltaTime * 5.0f;

    UpdateLightRays(deltaTime);
    UpdateGlowParticles(deltaTime, buttonRect);

    m_waveSpawnTimer += deltaTime;
    if (m_waveSpawnTimer >= 1.5f)
    {
        m_waveSpawnTimer = 0.0f;

        for (auto& wave : m_energyWaves)
        {
            if (!wave.active)
            {
                wave.active = true;
                wave.radius = 5.0f;
                wave.alpha = 0.6f * m_globalIntensity;
                break;
            }
        }
    }

    for (auto& wave : m_energyWaves)
    {
        if (wave.active)
        {
            wave.radius += wave.speed * deltaTime;
            wave.alpha *= (1.0f - deltaTime * 0.8f); // ����͸��

            if (wave.radius >= wave.maxRadius || wave.alpha <= 0.05f)
            {
                wave.active = false;
            }
        }
    }
}

void RadiantGlowAnimator::UpdateLightRays(float deltaTime)
{
    for (auto& ray : m_lightRays)
    {
        if (!ray.active) continue;

        float pulseValue = sin(m_totalTime * ray.pulsSpeed + ray.animOffset);
        ray.intensity = ray.baseIntensity + pulseValue * 0.3f * m_globalIntensity;
        ray.intensity = max(0.2f, min(1.0f, ray.intensity));

        float breathe = sin(m_totalTime * 1.5f + ray.animOffset) * 0.2f + 1.0f;
        ray.length = (25.0f + (ray.baseIntensity * 20.0f)) * breathe * m_globalIntensity;
    }
}

void RadiantGlowAnimator::UpdateGlowParticles(float deltaTime, const SDL_Rect& buttonRect)
{
    for (auto& particle : m_glowParticles)
    {
        if (!particle.active) continue;

        particle.angle += particle.speed * deltaTime;
        if (particle.angle >= 360.0f)
            particle.angle -= 360.0f;

        float pulse = sin(m_totalTime * 2.5f + particle.angle * M_PI / 180.0f) * 0.3f + 0.7f;
        particle.alpha = pulse * m_globalIntensity * 0.8f;

        particle.color.a = static_cast<Uint8>(particle.alpha * 255);

        float angleRad = particle.angle * M_PI / 180.0f;
        particle.x = cos(angleRad) * particle.distance;
        particle.y = sin(angleRad) * particle.distance;
    }
}

void RadiantGlowAnimator::Render(SDL_Renderer* renderer, const SDL_Rect& buttonRect)
{
    //RenderEnergyWaves(renderer, buttonRect);

    //RenderRotatingHalo(renderer, buttonRect);

 
     // RenderCenterGlow(renderer, buttonRect);

 
    RenderLightRays(renderer, buttonRect);

    //RenderGlowParticles(renderer, buttonRect);
}

void RadiantGlowAnimator::RenderLightRays(SDL_Renderer* renderer, const SDL_Rect& buttonRect)
{
    int centerX = buttonRect.x + buttonRect.w / 2;
    int centerY = buttonRect.y + buttonRect.h / 2;

    for (const auto& ray : m_lightRays)
    {
        if (!ray.active || ray.intensity <= 0.1f) continue;
        float totalAngle = (ray.angle + m_rotationAngle) * M_PI / 180.0f;
        int endX = centerX + static_cast<int>(cos(totalAngle) * ray.length);
        int endY = centerY + static_cast<int>(sin(totalAngle) * ray.length);
        SDL_Color startColor = ray.color;
        startColor.a = static_cast<Uint8>(255 * ray.intensity);
        SDL_Color endColor = ray.color;
        endColor.a = 0; 
        for (int i = 0; i < 3; ++i)
        {
            float layerIntensity = (3 - i) / 3.0f;
            SDL_Color layerStart = startColor;
            SDL_Color layerEnd = endColor;
            layerStart.a = static_cast<Uint8>(layerStart.a * layerIntensity);

            DrawGradientLine(renderer, centerX, centerY, endX, endY, layerStart, layerEnd, ray.thickness + i);
        }
    }
}

void RadiantGlowAnimator::RenderGlowParticles(SDL_Renderer* renderer, const SDL_Rect& buttonRect)
{
    int centerX = buttonRect.x + buttonRect.w / 2;
    int centerY = buttonRect.y + buttonRect.h / 2;

    for (const auto& particle : m_glowParticles)
    {
        if (!particle.active || particle.alpha <= 0.05f) continue;

        int particleX = centerX + static_cast<int>(particle.x);
        int particleY = centerY + static_cast<int>(particle.y);
        for (int layer = 0; layer < 3; ++layer)
        {
            int radius = static_cast<int>(particle.size + layer * 2);
            float layerAlpha = particle.alpha * (3 - layer) / 3.0f;

            SDL_Color layerColor = particle.color;
            layerColor.a = static_cast<Uint8>(layerAlpha * 255);

            DrawGlowCircle(renderer, particleX, particleY, radius, layerColor, layerAlpha);
        }
    }
}

void RadiantGlowAnimator::RenderCenterGlow(SDL_Renderer* renderer, const SDL_Rect& buttonRect)
{
    int centerX = buttonRect.x + buttonRect.w / 2;
    int centerY = buttonRect.y + buttonRect.h / 2;
    float coreIntensity = m_globalIntensity * (0.8f + sin(m_pulsePhase) * 0.2f);

    for (int layer = 0; layer < 5; ++layer)
    {
        int radius = 3 + layer * 2;
        float layerAlpha = coreIntensity * (5 - layer) / 5.0f * 0.6f;

        SDL_Color coreColor = m_glowColor;
        coreColor.a = static_cast<Uint8>(layerAlpha * 255);

        DrawGlowCircle(renderer, centerX, centerY, radius, coreColor, layerAlpha);
    }
}

void RadiantGlowAnimator::RenderRotatingHalo(SDL_Renderer* renderer, const SDL_Rect& buttonRect)
{
    int centerX = buttonRect.x + buttonRect.w / 2;
    int centerY = buttonRect.y + buttonRect.h / 2;

    for (size_t i = 0; i < m_haloRings.size(); ++i)
    {
        const auto& ring = m_haloRings[i];
        float ringRotation = m_totalTime * ring.rotationSpeed;
        for (int seg = 0; seg < ring.segmentCount; ++seg)
        {
            float segmentAngle = (360.0f / ring.segmentCount) * seg + ringRotation;
            float angleRad = segmentAngle * M_PI / 180.0f;

            int x = centerX + static_cast<int>(cos(angleRad) * ring.radius);
            int y = centerY + static_cast<int>(sin(angleRad) * ring.radius);
            SDL_Color segColor = ring.color;
            segColor.a = static_cast<Uint8>(ring.alpha * m_globalIntensity * 255);

            DrawGlowCircle(renderer, x, y, 2, segColor, ring.alpha * m_globalIntensity);
        }
    }
}

void RadiantGlowAnimator::RenderEnergyWaves(SDL_Renderer* renderer, const SDL_Rect& buttonRect)
{
    int centerX = buttonRect.x + buttonRect.w / 2;
    int centerY = buttonRect.y + buttonRect.h / 2;

    for (const auto& wave : m_energyWaves)
    {
        if (!wave.active || wave.alpha <= 0.05f) continue;
        SDL_Color waveColor = m_glowColor;
        waveColor.a = static_cast<Uint8>(wave.alpha * 255);
        const int pointCount = 32;
        for (int i = 0; i < pointCount; ++i)
        {
            float angle = (360.0f / pointCount) * i * M_PI / 180.0f;
            int x = centerX + static_cast<int>(cos(angle) * wave.radius);
            int y = centerY + static_cast<int>(sin(angle) * wave.radius);

            SDL_SetRenderDrawColor(renderer, waveColor.r, waveColor.g, waveColor.b, waveColor.a);
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

void RadiantGlowAnimator::DrawGradientLine(SDL_Renderer* renderer, int x1, int y1, int x2, int y2,
    const SDL_Color& startColor, const SDL_Color& endColor, float thickness)
{
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));

    if (steps == 0) return;

    float xStep = static_cast<float>(dx) / steps;
    float yStep = static_cast<float>(dy) / steps;

    for (int i = 0; i <= steps; ++i)
    {
        float t = static_cast<float>(i) / steps;
        SDL_Color currentColor = BlendColors(startColor, endColor, t);

        int x = x1 + static_cast<int>(i * xStep);
        int y = y1 + static_cast<int>(i * yStep);

        SDL_SetRenderDrawColor(renderer, currentColor.r, currentColor.g, currentColor.b, currentColor.a);
        int thicknessInt = static_cast<int>(thickness);
        for (int tx = -thicknessInt / 2; tx <= thicknessInt / 2; ++tx)
        {
            for (int ty = -thicknessInt / 2; ty <= thicknessInt / 2; ++ty)
            {
                SDL_RenderDrawPoint(renderer, x + tx, y + ty);
            }
        }
    }
}

void RadiantGlowAnimator::DrawGlowCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius,
    const SDL_Color& color, float intensity)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int y = -radius; y <= radius; ++y)
    {
        for (int x = -radius; x <= radius; ++x)
        {
            if (x * x + y * y <= radius * radius)
            {
                SDL_RenderDrawPoint(renderer, centerX + x, centerY + y);
            }
        }
    }
}

SDL_Color RadiantGlowAnimator::BlendColors(const SDL_Color& color1, const SDL_Color& color2, float t)
{
    return {
        static_cast<Uint8>(color1.r + (color2.r - color1.r) * t),
        static_cast<Uint8>(color1.g + (color2.g - color1.g) * t),
        static_cast<Uint8>(color1.b + (color2.b - color1.b) * t),
        static_cast<Uint8>(color1.a + (color2.a - color1.a) * t)
    };
}