SELECT
    m.MessageIndex,
    POWER(m.EncryptedMessage, k.d) % k.N AS Decrypted
FROM
    EncryptedMessages m
JOIN
    KeyPairs k ON m.KeyPairId = k.Id
