#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QAbstractListModel>
#include <QString>
#include <QList>

struct HistoryEntry {
    QString expression;
    QString result;
    bool isError;
};

class HistoryModel : public QAbstractListModel {
    Q_OBJECT

public:
    enum Roles {
        ExpressionRole = Qt::UserRole + 1,
        ResultRole,
        IsErrorRole
    };

    explicit HistoryModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void addEntry(const QString &expression, const QString &result, bool isError = false);
    void clear();

    QString lastResult() const;
    int entryCount() const;

private:
    QList<HistoryEntry> m_entries;
};

#endif // HISTORYMODEL_H
