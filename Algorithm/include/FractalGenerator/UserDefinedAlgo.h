#pragma once
#include "FractalAlgo.h"

class UserDefinedAlgo final : public FractalAlgo
{
public:
    UserDefinedAlgo() = default;
    UserDefinedAlgo(std::string code);
    ~UserDefinedAlgo() = default;

    int ProcessCoord(const DataCoord& coord) override;
    boost::compute::function<int(DataCoord)> GetProcessCoordGPU() override;

    bool IsFunctionValid() { return m_is_valid; }
    void UpdateFunctionCode(const std::string& code);
private:
    bool IsSameFunction(const std::string& code) { return code == m_code; };
    void ValidateFunction();
    std::string m_code;
    bool m_is_valid = false;
    bool m_is_function_ready = false;
};

