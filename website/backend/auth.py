import base64
import json

from webauthn import (
    generate_registration_options,
    verify_registration_response,
    generate_authentication_options,
    verify_authentication_response,
    options_to_json,
    base64url_to_bytes,
)

from webauthn.helpers.structs import (
    AuthenticatorSelectionCriteria,
    ResidentKeyRequirement,
    UserVerificationRequirement,
    PublicKeyCredentialDescriptor,
)


RP_ID = "localhost"
RP_NAME = "Smart Door Access System"
ORIGIN = "http://localhost:5000"


def encode_bytes(value):
    return base64.urlsafe_b64encode(value).rstrip(b"=").decode("utf-8")


def create_registration_options(username, user_id, existing_credentials=None):
    if existing_credentials is None:
        existing_credentials = []

    excluded_credentials = [
        PublicKeyCredentialDescriptor(
            id=base64url_to_bytes(item)
        )
        for item in existing_credentials
    ]

    options = generate_registration_options(
        rp_id=RP_ID,
        rp_name=RP_NAME,
        user_id=user_id,
        user_name=username,
        user_display_name=username,
        authenticator_selection=AuthenticatorSelectionCriteria(
            resident_key=ResidentKeyRequirement.REQUIRED,
            user_verification=UserVerificationRequirement.REQUIRED,
        ),
        exclude_credentials=excluded_credentials,
    )

    return {
        "options": json.loads(options_to_json(options)),
        "challenge": encode_bytes(options.challenge),
    }


def verify_registration(credential, expected_challenge):
    verification = verify_registration_response(
        credential=credential,
        expected_challenge=base64url_to_bytes(expected_challenge),
        expected_origin=ORIGIN,
        expected_rp_id=RP_ID,
        require_user_verification=True,
    )

    return {
        "credential_id": encode_bytes(verification.credential_id),
        "public_key": encode_bytes(verification.credential_public_key),
        "sign_count": verification.sign_count,
    }


def create_authentication_options(credentials):
    allowed_credentials = [
        PublicKeyCredentialDescriptor(
            id=base64url_to_bytes(item)
        )
        for item in credentials
    ]

    options = generate_authentication_options(
        rp_id=RP_ID,
        allow_credentials=allowed_credentials,
        user_verification=UserVerificationRequirement.REQUIRED,
    )

    return {
        "options": json.loads(options_to_json(options)),
        "challenge": encode_bytes(options.challenge),
    }


def verify_authentication(credential, expected_challenge, stored_credential):
    verification = verify_authentication_response(
        credential=credential,
        expected_challenge=base64url_to_bytes(expected_challenge),
        expected_origin=ORIGIN,
        expected_rp_id=RP_ID,
        credential_public_key=base64url_to_bytes(
            stored_credential["public_key"]
        ),
        credential_current_sign_count=stored_credential["sign_count"],
        require_user_verification=True,
    )

    return {
        "new_sign_count": verification.new_sign_count,
    }
