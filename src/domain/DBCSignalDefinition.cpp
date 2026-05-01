#include "vehicle-sim/domain/DBCSignalDefinition.h"
#include "vehicle-sim/domain/DBCParser.h"

namespace vehicle_sim::domain {

DBCSignalDefinition::DBCSignalDefinition(
    const DBCSignalParams& params
) noexcept
    : canId(params.canId)
    , name(params.name)
    , startBit(params.startBit)
    , bitLength(params.bitLength)
    , byteOrder(params.byteOrder)
    , scale(params.scale)
    , offset(params.offset)
    , isSigned(params.isSigned)
    , unit(params.unit)
    , min(params.min)
    , max(params.max)
    , valueTable(params.valueTable)
{}

DBCSignalDefinition::DBCSignalDefinition(
    std::uint16_t canId,
    std::string name,
    std::size_t startBit,
    std::size_t bitLength,
    DBCByteOrder byteOrder,
    double scale,
    double offset,
    bool isSigned,
    std::string unit,
    double min,
    double max,
    std::vector<DBCValueEntry> valueTable
) noexcept
    : DBCSignalDefinition(DBCSignalParams{
        canId,
        std::move(name),
        startBit,
        bitLength,
        byteOrder,
        scale,
        offset,
        isSigned,
        std::move(unit),
        min,
        max,
        std::move(valueTable)
    })
{}

bool DBCSignalDefinition::operator==(
    const DBCSignalDefinition& other
) const noexcept {
    if (canId != other.canId
        || name != other.name
        || startBit != other.startBit
        || bitLength != other.bitLength
        || byteOrder != other.byteOrder
        || scale != other.scale
        || offset != other.offset
        || isSigned != other.isSigned
        || unit != other.unit
        || min != other.min
        || max != other.max) {
        return false;
    }

    if (valueTable.size() != other.valueTable.size()) {
        return false;
    }

    for (std::size_t i = 0; i < valueTable.size(); ++i) {
        if (valueTable[i].value != other.valueTable[i].value
            || valueTable[i].description != other.valueTable[i].description) {
            return false;
        }
    }

    return true;
}

bool DBCSignalDefinition::operator!=(
    const DBCSignalDefinition& other
) const noexcept {
    return !(*this == other);
}

const std::vector<DBCSignalDefinition>* DBCParseResult::getSignalsForCanId(
    std::uint16_t canId
) const noexcept {
    auto it = signalsByCanId.find(canId);
    return it != signalsByCanId.end() ? &it->second : nullptr;
}

std::size_t DBCParseResult::totalSignalCount() const noexcept {
    std::size_t count = 0;
    for (const auto& [_, signals] : signalsByCanId) {
        count += signals.size();
    }
    return count;
}

} // namespace vehicle_sim::domain
