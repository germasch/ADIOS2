#include "Null2Writer.h"
#include "Null2Writer.tcc"

namespace adios2
{
namespace core
{
namespace engine
{

struct Null2Writer::Null2WriterImpl
{
    size_t CurrentStep = 0;
    bool IsInStep = false;
    bool IsOpen = true;
};

Null2Writer::Null2Writer(IO &io, const std::string &name, const Mode mode,
                         MPI_Comm mpiComm)
: Engine("Null2Writer", io, name, mode, mpiComm),
  Impl(new Null2Writer::Null2WriterImpl)
{
}

Null2Writer::~Null2Writer() = default;

StepStatus Null2Writer::BeginStep(StepMode mode, const float timeoutSeconds)
{
    if (!Impl->IsOpen)
    {
        throw std::runtime_error(
            "ERROR: Null2Writer::BeginStep: Engine already closed");
    }

    if (Impl->IsInStep)
    {
        throw std::runtime_error(
            "ERROR: Null2Writer::BeginStep: Step already active");
    }

    Impl->IsInStep = true;
    ++Impl->CurrentStep;
    return StepStatus::OK;
}

size_t Null2Writer::CurrentStep() const
{
    if (!Impl->IsOpen)
    {
        throw std::runtime_error(
            "ERROR: Null2Writer::CurrentStep: Engine already closed");
    }

    return Impl->CurrentStep;
}

void Null2Writer::EndStep()
{
    if (!Impl->IsOpen)
    {
        throw std::runtime_error(
            "ERROR: Null2Writer::EndStep: Engine already closed");
    }

    if (!Impl->IsInStep)
    {
        throw std::runtime_error("ERROR: Null2Writer::EndStep: No active step");
    }

    Impl->IsInStep = false;
}

void Null2Writer::PerformPuts()
{
    if (!Impl->IsOpen)
    {
        throw std::runtime_error(
            "ERROR: Null2Writer::PerformPuts: Engine already closed");
    }

    return;
}

void Null2Writer::Flush(const int)
{
    if (!Impl->IsOpen)
    {
        throw std::runtime_error(
            "ERROR: Null2Writer::Flush: Engine already closed");
    }

    return;
}

void Null2Writer::DoClose(const int)
{
    if (!Impl->IsOpen)
    {
        throw std::runtime_error("ERROR: Null2Writer::DoClose: already closed");
    }

    Impl->IsOpen = false;
}

} // end namespace engine
} // end namespace core
} // end namespace adios2
