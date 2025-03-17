#pragma once

#include "CoreMinimal.h"

using errInteger32 = int32;

template <errInteger32 errValue>
struct TErrNo
{
    static constexpr    errInteger32 _no = errValue;
    static const        FString _text;
};

#define ErrNoText(No, Sign, Text) \
    constexpr errInteger32 Sign##Value = No; \
    template <> const FString TErrNo<Sign##Value>::_text = TEXT(Text); \
    constexpr auto Sign = std::integral_constant<errInteger32, Sign##Value>{};

//  ====================================== 에러? ======================================

ErrNoText(0,    errNotError,                    "Success")
ErrNoText(1,    errInValidUserObject,           "유효하지 않은 UserObject입니다.")
ErrNoText(2,    errInValidLogin,                "비정상적인 로그인입니다.")
ErrNoText(3,    errInValidUserID,               "유효하지 않은 UserID입니다.")
ErrNoText(4,    errFailedExecuteQuery,          "쿼리 실행 중 실패하였습니다.")
ErrNoText(5,    errFailedApplyQueryResult,      "쿼리 결과 적용 중 실패하였습니다.")
ErrNoText(6,    errNotEnoughInventory,          "인벤토리 공간이 부족합니다.")
ErrNoText(7,    errFailedReward,                "보상 지급에 실패하였습니다.")
ErrNoText(8,    errNotServer,                   "서버에서만 호출해야될 함수입니다.")
ErrNoText(9,    errFailedApplyMemory,           "메모리 적용에 실패하였습니다. 재 로그인 시 정상 반영됩니다.")
ErrNoText(10,   errAlreadyAcceptQuest,          "이미 존재하는 퀘스트입니다.")
ErrNoText(11,   errInValidRequest,              "유효하지 않은 요청입니다.")
ErrNoText(12,   errFailedVerification,          "검증에 실패하였습니다.")

//  ==================================================================================

// 조금 원하는 형태가 아닌듯... 
// FErrNo가 Int만 들고있고 프로젝트 시작 시 Map을 할당하여 Value로 Text를 들고있게 하는게 좋으려나?

typedef struct FErrNo
{
    FErrNo() : FErrNo(errNotError) {}
    FErrNo(errInteger32) : FErrNo(errNotError) {}
    FErrNo(FErrNo&& other) noexcept
        : _text(other._text)
        , _no(other._no)
    {}

    template <errInteger32 no>
    constexpr FErrNo(std::integral_constant<errInteger32, no>)
        : _text(&TErrNo<no>::_text) 
        , _no(TErrNo<no>::_no)
    {}

    template <errInteger32 no>
    void operator=(std::integral_constant<errInteger32, no>)
    {
        _text = &TErrNo<no>::_text;
        _no   = TErrNo<no>::_no;
    }

    const FString& GetText() const { return *_text; }
    const bool IsFailed() const { return _no != 0; }

    FErrNo(const FErrNo&) = delete;
    FErrNo& operator=(const FErrNo&) = delete;
    FErrNo& operator=(FErrNo&& other) = default;

private:
    const FString*  _text;
    int32           _no;
} ErrNo;