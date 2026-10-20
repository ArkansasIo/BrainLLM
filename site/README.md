# BrainLLM GitHub Pages Workspace

This directory contains the static developer workspace deployed by `.github/workflows/pages.yml`.

## Included panels

- Overview and high-level architecture map
- AI Playground with clearly labelled browser demo responses
- Optional connection to a running BrainLLM REST API
- Agent workflow illustration
- Browser-local personality presets
- API examples and links to the source documentation

## Deploy

The Pages workflow validates the static files on pushes and pull requests that touch `site/` or the workflow file. Pushes to `main` deploy `site/` as the Pages artifact. In the repository settings, set **Settings → Pages → Build and deployment → Source** to **GitHub Actions** if it is not already selected. The workflow requires Pages deployment permissions configured by GitHub.

## Backend connection

GitHub Pages is static hosting; it does not run the native C++/Qt application. To use live inference, run the API server separately and configure its base origin in the playground. The server must enable a suitable CORS policy. Do not publish private API tokens in client-side code or expose an unauthenticated API to the public internet.
