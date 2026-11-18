// SPDX-License-Identifier: GPL-3.0-or-later
#include "MobileProtocol.h"
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QStringDecoder>
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <memory>


namespace {
#define REQUIRE(value) do { if (!(value)) return std::nullopt; } while (false)
std::optional<QString> text(const QByteArray &bytes, qsizetype limit, bool empty = true) {
    QStringDecoder decoder(QStringDecoder::Utf8);
    const QString value = decoder.decode(bytes);
    REQUIRE(!decoder.hasError() && (empty || !value.isEmpty()) && value.size() <= limit);
    for (const QChar c : value) REQUIRE(c.category() != QChar::Other_Control);
    return value;
}
QByteArray mac(const QByteArray &key, const QByteArray &value) {
    return QMessageAuthenticationCode::hash(value, key, QCryptographicHash::Sha256);
}
std::optional<QByteArray> decode(const QByteArray &value) {
    auto decoded = QByteArray::fromBase64Encoding(value, QByteArray::AbortOnBase64DecodingErrors);
    REQUIRE(bool(decoded)); return decoded.decoded;
}
}
bool MobileProtocol::validKey(const QString &key) {
    static const QRegularExpression pattern(QStringLiteral("^[a-f0-9]{64}$"));
    return pattern.match(key).hasMatch();
}
QByteArray MobileProtocol::newChallenge() {
    QByteArray random(32, Qt::Uninitialized);
    for (qsizetype i = 0; i < random.size(); ++i) random[i] = char(QRandomGenerator::system()->generate() & 255);
    return random.toHex();
}
std::optional<MobileMessage> MobileProtocol::verify(const QString &key, const QByteArray &challenge, const QByteArray &line) {
    REQUIRE(validKey(key) && validKey(QString::fromLatin1(challenge)) && line.size() <= 4096);
    const auto fields = line.split('\t');
    REQUIRE(fields.size() == 6 && (fields[0] == "v1" || fields[0] == "v2"));
    static const QRegularExpression id(QStringLiteral("^[A-Za-z0-9-]{1,64}$"));
    REQUIRE(id.match(QString::fromLatin1(fields[1])).hasMatch() && id.match(QString::fromLatin1(fields[3])).hasMatch());
    REQUIRE(fields[2] == "RING" || fields[2] == "CONNECT" || fields[2] == "END" || fields[2] == "TEST");
    REQUIRE(validKey(QString::fromLatin1(fields[5])));
    const QByteArray secret = QByteArray::fromHex(key.toLatin1());
    const QByteArray payload = line.left(line.lastIndexOf('\t'));
    const QByteArray expected = mac(secret, challenge + '\n' + payload).toHex();
    REQUIRE(CRYPTO_memcmp(expected.constData(), fields[5].constData(), size_t(expected.size())) == 0);
    MobileMessage result{QString::fromLatin1(fields[1]),QString::fromLatin1(fields[2]),QString::fromLatin1(fields[3]),{},{},{}};
    const auto decoded = decode(fields[4]); REQUIRE(decoded);
    QByteArray bytes = *decoded;
    if (fields[0] == "v1") { const auto label = text(bytes,60,false); REQUIRE(label); result.label = *label; return result; }
    REQUIRE(bytes.size() >= 28);
    const QByteArray derived = mac(secret, QByteArrayLiteral("ScreenRing v2 payload"));
    const QByteArray iv = bytes.left(12), tag = bytes.right(16), ciphertext = bytes.mid(12,bytes.size()-28);
    const QByteArray aad = challenge + '\n' + fields[0] + '\t' + fields[1] + '\t' + fields[2] + '\t' + fields[3];
    std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)> ctx(EVP_CIPHER_CTX_new(),EVP_CIPHER_CTX_free);
    REQUIRE(bool(ctx));
    REQUIRE(EVP_DecryptInit_ex(ctx.get(),EVP_aes_256_gcm(),nullptr,nullptr,nullptr)==1);
    REQUIRE(EVP_CIPHER_CTX_ctrl(ctx.get(),EVP_CTRL_GCM_SET_IVLEN,12,nullptr)==1);
    REQUIRE(EVP_DecryptInit_ex(ctx.get(),nullptr,nullptr,reinterpret_cast<const unsigned char*>(derived.constData()),reinterpret_cast<const unsigned char*>(iv.constData()))==1);
    int count=0;
    REQUIRE(EVP_DecryptUpdate(ctx.get(),nullptr,&count,reinterpret_cast<const unsigned char*>(aad.constData()),int(aad.size()))==1);
    QByteArray plain(ciphertext.size()+16,Qt::Uninitialized);
    REQUIRE(EVP_DecryptUpdate(ctx.get(),reinterpret_cast<unsigned char*>(plain.data()),&count,reinterpret_cast<const unsigned char*>(ciphertext.constData()),int(ciphertext.size()))==1);
    int length=count;
    REQUIRE(EVP_CIPHER_CTX_ctrl(ctx.get(),EVP_CTRL_GCM_SET_TAG,16,const_cast<char*>(tag.constData()))==1);
    REQUIRE(EVP_DecryptFinal_ex(ctx.get(),reinterpret_cast<unsigned char*>(plain.data())+length,&count)==1);
    plain.resize(length+count);
    const auto caller=plain.split('\n'); REQUIRE(caller.size()==3);
    const auto label=text(caller[0],60,false), name=text(caller[1],120), number=text(caller[2],64);
    REQUIRE(label && name && number);
    result.label=*label;result.name=*name;result.number=*number;
    return result;
}

#undef REQUIRE
