#pragma once

#include <array>
struct Colour {
    unsigned char r;
    unsigned char g;
    unsigned char b;
        
    Colour(unsigned char r, unsigned char g, unsigned char b);
    Colour() : Colour(0,0,0) {}

    std::array<float, 3> GetFltArr() const
    {
        return {
            static_cast<float>(r) / 255.f,
            static_cast<float>(g) / 255.f,
            static_cast<float>(b) / 255.f,
        };
    } 
    
    void SetFromFltArr(const std::array<float, 3>& input)
    {
        r = input[0] * 255.f;
        g = input[1] * 255.f;
        b = input[2] * 255.f;
    }

    friend bool operator==(const Colour& f, const Colour& s)
    {
        return f.r == s.r && f.g == s.g && f.b == s.b;
    }

    friend bool operator!=(const Colour& f, const Colour& s)
    {
        return f.r != s.r || f.g != s.g || f.b != s.b;
    }

    friend Colour operator-(const Colour& first, const Colour& second)
    {
        return Colour(first.r - second.r, first.g - second.g, first.b - second.b);
    }

    friend Colour operator+(const Colour& first, const Colour& second)
    {
        return Colour(first.r + second.r, first.g + second.g, first.b + second.b);
    }
};
    