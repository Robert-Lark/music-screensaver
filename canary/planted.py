"""The canary: one planted violation for ruff. Never import this file."""

import os


def shares_a_default(items: list[str] = []) -> list[str]:
    return items
