#pragma once

#include <QAbstractTableModel>
#include "SimulationData.h"

class ResultsModel : public QAbstractTableModel {
    Q_OBJECT
public:
    enum TaskType { TestTask, MainTask };

    ResultsModel(TaskType type, QObject *parent = nullptr)
        : QAbstractTableModel(parent), m_type(type) {}

    void setResults(const SimulationResults* results) {
        beginResetModel();
        m_data = results;
        endResetModel();
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        if (!m_data) return 0;
        return m_data->steps.size();
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        return (m_type == TestTask) ? 11 : 9;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        if (!m_data || !index.isValid()) return QVariant();

        if (role == Qt::DisplayRole) {
            const auto& step = m_data->steps[index.row()];

            auto fmt = [](double v) { return QString::number(v, 'f', 15); };

            switch (index.column()) {
            case 0: return step.iter;
            case 1: return fmt(step.x);
            case 2: return fmt(step.v.data[0]);
            case 3: return fmt(step.v2.data[0]);
            case 4: return fmt(step.v.data[0] - step.v2.data[0]); // v - v2
            case 5: return fmt(step.olp);
            case 6: return fmt(step.h);
            case 7: return step.c1;
            case 8: return step.c2;
            case 9: {
                if (m_type == TestTask && index.row() < m_data->exactValues.size())
                    return fmt(m_data->exactValues[index.row()]);
                return QVariant();
            }
            case 10: {
                if (m_type == TestTask && index.row() < m_data->exactValues.size()) {
                    double exact = m_data->exactValues[index.row()];
                    return fmt(std::abs(step.v.data[0] - exact));
                }
                return QVariant();
            }
            }
        }

        if (role == Qt::TextAlignmentRole) {
            return Qt::AlignCenter;
        }
        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
            static const QStringList headersTest = {
                "i",
                "xᵢ",
                "vᵢ",
                "v₂ᵢ",
                "vᵢ - v₂ᵢ",
                "OLP",
                "hᵢ",
                "C1",
                "C2",
                "uᵢ",
                "|uᵢ - vᵢ|"
            };

            static const QStringList headersMain = {
                "i",
                "xᵢ",
                "vᵢ",
                "v₂ᵢ",
                "vᵢ - v₂ᵢ",
                "OLP",
                "hᵢ",
                "C1",
                "C2",
            };

            const QStringList& h = (m_type == TestTask) ? headersTest : headersMain;
            if (section < h.size()) return h[section];
        }
        return QVariant();
    }

private:
    const SimulationResults* m_data = nullptr;
    TaskType m_type;
};
