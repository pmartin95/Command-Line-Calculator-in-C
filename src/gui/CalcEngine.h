#ifndef CALCENGINE_H
#define CALCENGINE_H

#include <QObject>
#include <QString>

// Include mpfr.h/gmp.h first outside extern "C" so GMP's C++ operator
// overloads compile correctly, then include the project's C headers.
#include <mpfr.h>

extern "C" {
#include "formatter.h"
}

class CalcEngine : public QObject {
    Q_OBJECT

public:
    explicit CalcEngine(QObject *parent = nullptr);
    ~CalcEngine();

    QString evaluate(const QString &expression);

    void setPrecision(int bits);
    int precision() const;
    int decimalDigits() const;

    void setFormat(NumberFormat format);
    NumberFormat format() const;

    bool hasError() const;
    QString lastError() const;

    QString lastResult() const;

signals:
    void precisionChanged(int bits);
    void formatChanged(NumberFormat format);

private:
    bool m_hasError;
    QString m_lastError;
    QString m_lastResult;
};

#endif // CALCENGINE_H
