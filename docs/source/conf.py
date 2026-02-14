# Configuration file for the Sphinx documentation builder.
import os
import sys

# Make sure autodoc can find your modules
sys.path.insert(0, os.path.abspath('../..'))

# -- Project information -----------------------------------------------------
project = 'Vesper'
author = 'Xenon'
release = ''
copyright = '2026, Xenon'

# -- General configuration ---------------------------------------------------
extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.viewcode',
    'sphinx.ext.napoleon',
]

templates_path = ['_templates']
exclude_patterns = []

# -- HTML output options -----------------------------------------------------
html_theme = 'furo'

# Add custom CSS if needed
html_static_path = ['_static']
