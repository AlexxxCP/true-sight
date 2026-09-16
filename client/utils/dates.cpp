#include "utils/dates.hpp"

#include <stdexcept>

namespace utils {

QDateTime parseDate(const QString& rawValue)
{
    QString isoValue = rawValue.trimmed();
    isoValue.replace(' ', 'T');

    // QDateTime stores millisecond precision; the API may return
    // PostgreSQL timestamps with six fractional digits.
    const qsizetype dot = isoValue.indexOf('.');
    const qsizetype timezone = isoValue.indexOf('+', dot);
    if (dot >= 0 && timezone > dot) {
        const QString milliseconds =
            isoValue.mid(dot + 1, timezone - dot - 1)
                .leftJustified(3, '0')
                .left(3);
        isoValue = isoValue.left(dot + 1)
            + milliseconds
            + isoValue.mid(timezone);
    }

    if (isoValue.endsWith("+00"))
        isoValue += ":00";

    const QDateTime result =
        QDateTime::fromString(isoValue, Qt::ISODateWithMs);
    if (!result.isValid())
        throw std::runtime_error("Invalid created_at timestamp");

    return result;
}

} // namespace utils
