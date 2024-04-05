# Fonction pour calculer l'exponentiation modulaire efficace
def decrypt_message(encrypted_msg, d, n):
    return pow(encrypted_msg, d, n)


# Définissons les données de chaque colonne en tant que listes
encrypted_messages = [4284104, 5173926, 2439879, 11347151, 8083188, 12683967, 1452637, 5021529, 8843626, 6443957, 2103639, 10521358, 11177625, 5437345, 13739409]
ds = [5364235, 8027023, 3379435, 4640321, 6475317, 7913225, 9844693, 4606669, 77351, 1693433, 4583261, 491311, 7664975, 9673633, 7803361]
ns = [11726027, 10657873, 12372079, 10358239, 13444163, 15533527, 9648449, 12261979, 10725733, 16080019, 12422489, 11882051, 11579539, 11577427, 11788501]
e = [11554943, 9660109, 11551409, 10076293, 10092983, 14805481, 9625789, 8678863, 9362921, 12923297, 11545577, 8691269, 11546371, 10343167, 9317299]
p = [2711, 3253, 4013, 3779, 4073, 3469, 3319, 3391, 3727, 2749, 3307, 4001, 3967, 3547, 4091]
q = [3559, 3559, 3083, 2741, 2843, 3581, 3533, 3163, 3163, 3877, 3593, 4019, 3389, 3457, 3797]

def brute_force_decrypt(encrypted_messages, ds, ns):
    # Stocker les résultats dans un dictionnaire avec l'index du message comme clé
    # et le caractère déchiffré comme valeur
    results = {}

    for message_index, encrypted_msg in enumerate(encrypted_messages):
        for d in ds:
            for n in ns:
                decrypted_number = decrypt_message(encrypted_msg, d, n)
                # Vérifier si le nombre déchiffré correspond à un caractère ASCII valide
                if decrypted_number < 256:
                    # Ajouter le caractère déchiffré au dictionnaire des résultats
                    results[message_index] = chr(decrypted_number)
                    # Print ds index and ns index
                    print(f"ds index: {ds.index(d)}, ns index: {ns.index(n)}")
                    break  # Arrêter la recherche une fois qu'un caractère valide est trouvé

            if message_index in results:
                print(f"Message {message_index}: {results[message_index]}")
                break  # Arrêter la recherche pour ce message si un caractère valide a été trouvé

    return results

# Exécuter la fonction brute force
decrypted_results = brute_force_decrypt(encrypted_messages, ds, ns)

for message_index, decrypted_char in decrypted_results.items():
    print(f"Message {message_index}: {decrypted_char}")

# -- print columns of the EncryptedMessages table
# SELECT * FROM EncryptedMessages;

# Expected columns: MESSAGEINDEX, DECRYPTED
# |EncryptedMessages                |PrivateKe| PublicKeys          | PrimeFactors|
# ---------------------------------------------------------------------------------
# | MESSAGEINDEX | ENCRYPTEDMESSAGE | D       | N        | E        | P    | Q    |
# ---------------------------------------------------------------------------------
# | 0            | 4284104          | 5364235 | 11726027 | 11554943 | 2711 | 3559 |
# | 1            | 5173926          | 8027023 | 10657873 | 9660109  | 3253 | 3559 |
# | 2            | 2439879          | 3379435 | 12372079 | 11551409 | 4013 | 3083 |
# | 3            | 11347151         | 4640321 | 10358239 | 10076293 | 3779 | 2741 |
# | 4            | 8083188          | 6475317 | 13444163 | 10092983 | 4073 | 2843 |
# | 5            | 12683967         | 7913225 | 15533527 | 14805481 | 3469 | 3581 |
# | 6            | 1452637          | 9844693 | 9648449  | 9625789  | 3319 | 3533 |
# | 7            | 5021529          | 4606669 | 12261979 | 8678863  | 3391 | 3163 |
# | 8            | 8843626          | 77351   | 10725733 | 9362921  | 3727 | 3163 |
# | 9            | 6443957          | 1693433 | 16080019 | 12923297 | 2749 | 3877 |
# | 10           | 2103639          | 4583261 | 12422489 | 11545577 | 3307 | 3593 |
# | 11           | 10521358         | 491311  | 11882051 | 8691269  | 4001 | 4019 |
# | 12           | 11177625         | 7664975 | 11579539 | 11546371 | 3967 | 3389 |
# | 13           | 5437345          | 9673633 | 11577427 | 10343167 | 3547 | 3457 |
# | 14           | 13739409         | 7803361 | 11788501 | 9317299  | 4091 | 3797 |
# --------------------------------------------------------------------------------- 

# SELECT * FROM PublicKeys;

# Expected columns: MESSAGEINDEX, DECRYPTED
# -----------------------
# | N        | E        |
# -----------------------
# | 11726027 | 11554943 |
# | 10657873 | 9660109  |
# | 12372079 | 11551409 |
# | 10358239 | 10076293 |
# | 13444163 | 10092983 |
# | 15533527 | 14805481 |
# | 9648449  | 9625789  |
# | 12261979 | 8678863  |
# | 10725733 | 9362921  |
# | 16080019 | 12923297 |
# | 12422489 | 11545577 |
# | 11882051 | 8691269  |
# | 11579539 | 11546371 |
# | 11577427 | 10343167 |
# | 11788501 | 9317299  |
# -----------------------

# SELECT * FROM PrivateKeys;
# 1 column returned but 2 columns expected
# Expected columns: MESSAGEINDEX, DECRYPTED
# -----------
# | D       |
# -----------
# | 5364235 |
# | 8027023 |
# | 3379435 |
# | 4640321 |
# | 6475317 |
# | 7913225 |
# | 9844693 |
# | 4606669 |
# | 77351   |
# | 1693433 |
# | 4583261 |
# | 491311  |
# | 7664975 |
# | 9673633 |
# | 7803361 |
# -----------

# SELECT * FROM PrimeFactors;
# Expected columns: MESSAGEINDEX, DECRYPTED
# ---------------
# | P    | Q    |
# ---------------
# | 2711 | 3559 |
# | 3253 | 3559 |
# | 4013 | 3083 |
# | 3779 | 2741 |
# | 4073 | 2843 |
# | 3469 | 3581 |
# | 3319 | 3533 |
# | 3391 | 3163 |
# | 3727 | 3163 |
# | 2749 | 3877 |
# | 3307 | 3593 |
# | 4001 | 4019 |
# | 3967 | 3389 |
# | 3547 | 3457 |
# | 4091 | 3797 |
# ---------------