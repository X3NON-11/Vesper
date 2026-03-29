#!/usr/bin/env bash

if [ ! -d ".venv" ]; then
  python3 -m venv .venv
fi

source .venv/bin/activate
pip install -q sphinx sphinx-autobuild furo

xdg-open http://localhost:8000 &
sphinx-autobuild source build/html