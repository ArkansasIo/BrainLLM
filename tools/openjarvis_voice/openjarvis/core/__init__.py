"""Minimal OpenJarvis core exports required by the vendored voice subset."""

from openjarvis.core.registry import SpeechRegistry, ToolRegistry, TTSRegistry
from openjarvis.core.types import ToolResult

__all__ = ["SpeechRegistry", "ToolRegistry", "TTSRegistry", "ToolResult"]
