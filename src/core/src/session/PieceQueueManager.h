#pragma once

#include "collection/ThreadSafeQueue.h"

namespace core
{
class PieceQueueManager
{
public:
    explicit PieceQueueManager(libs::collection::ThreadSafeQueue<int>& piecesQueue);

    void addPieceIdoQueue(const int& pieceId);

    std::optional<int> getPieceIdFromQueue() const;

private:
    libs::collection::ThreadSafeQueue<int>& piecesQueue;
    mutable std::mutex lock;
};
}