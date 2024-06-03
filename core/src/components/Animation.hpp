//
// Created by Steve Wheeler on 06/04/2024.
//

#pragma once

namespace sage
{
struct Animation
{
    ModelAnimation* animations;
    Model* model;
    unsigned int animIndex = 0;
    unsigned int animCurrentFrame = 0;
    int animsCount;
    bool oneShot = false;
    
    Animation(const char* _modelPath, Model* _model) :
    model(_model)
    {
        animsCount = 0;
        animations = LoadModelAnimations(_modelPath, &animsCount);
        animIndex = 0;
    }

    Animation(const Animation&) = delete;
    Animation& operator=(const Animation&) = delete;
    
    ~Animation()
    {
        UnloadModelAnimations(animations, animsCount);
    }

    void ChangeAnimation(int index, bool _oneShot = false)
    {
        animIndex = index;
        if (_oneShot)
        {
            oneShot = true;
            animCurrentFrame = 0;
        }
    }
};
}