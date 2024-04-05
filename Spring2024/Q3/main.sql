SELECT newColor AS color, COUNT(*) AS count
FROM pixels
INNER JOIN pixelUpdates ON pixels.id = pixelUpdates.pixelId
WHERE pixelUpdates.updatedAt > pixels.firstPaintedAt
GROUP BY newColor
ORDER BY count DESC;