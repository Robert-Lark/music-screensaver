#!/bin/bash
# The canary: one planted violation for shellcheck (an unquoted variable). Never run this file.
name="two words"
ls $name
