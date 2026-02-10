#include "HistoryModel.h"
#include <QColor>
#include <QFont>

HistoryModel::HistoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_entries.size();
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};

    const HistoryEntry &entry = m_entries.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return QStringLiteral("> %1\n= %2").arg(entry.expression, entry.result);
    case Qt::ForegroundRole:
        if (entry.isError)
            return QColor(Qt::red);
        return {};
    case Qt::FontRole: {
        QFont font;
        font.setFamily(QStringLiteral("monospace"));
        return font;
    }
    case ExpressionRole:
        return entry.expression;
    case ResultRole:
        return entry.result;
    case IsErrorRole:
        return entry.isError;
    }

    return {};
}

void HistoryModel::addEntry(const QString &expression, const QString &result, bool isError)
{
    beginInsertRows(QModelIndex(), m_entries.size(), m_entries.size());
    m_entries.append({expression, result, isError});
    endInsertRows();
}

void HistoryModel::clear()
{
    beginResetModel();
    m_entries.clear();
    endResetModel();
}

QString HistoryModel::lastResult() const
{
    if (m_entries.isEmpty())
        return QStringLiteral("0");

    // Find last non-error result
    for (int i = m_entries.size() - 1; i >= 0; --i) {
        if (!m_entries[i].isError)
            return m_entries[i].result;
    }
    return QStringLiteral("0");
}

int HistoryModel::entryCount() const
{
    return m_entries.size();
}
