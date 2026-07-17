#!/usr/bin/env python3
"""
Kodi Addon Validator for visualization.matrix
Validates addon.xml, Kodi API usage, and visualization-specific requirements.
"""

import os
import sys
import re
import xml.etree.ElementTree as ET
from pathlib import Path


class KodiAddonValidator:
    def __init__(self):
        self.errors = []
        self.warnings = []
        self.info = []

    def validate_addon_xml(self):
        """Validate addon.xml or addon.xml.in structure."""
        xml_files = ['addon.xml', 'visualization.matrix/addon.xml']
        xml_in_files = ['addon.xml.in', 'visualization.matrix/addon.xml.in']
        
        found = False
        for xml_file in xml_files + xml_in_files:
            if os.path.exists(xml_file):
                found = True
                self._validate_xml_file(xml_file)
                break
        
        if not found:
            self.errors.append("No addon.xml or addon.xml.in found")

    def _validate_xml_file(self, xml_file):
        """Validate a single XML file."""
        try:
            tree = ET.parse(xml_file)
            root = tree.getroot()
        except ET.ParseError as e:
            self.errors.append(f"Invalid XML in {xml_file}: {str(e)}")
            return

        # Check required attributes
        required_attrs = ['id', 'version', 'name', 'provider']
        for attr in required_attrs:
            if attr not in root.attrib:
                self.errors.append(f"Missing required attribute '{attr}' in {xml_file}")

        # Validate addon id
        addon_id = root.attrib.get('id', '')
        if addon_id != 'visualization.matrix':
            self.warnings.append(f"Unexpected addon id: '{addon_id}' (expected 'visualization.matrix')")

        # Validate version format (X.Y.Z)
        version = root.attrib.get('version', '')
        if not re.match(r'^\d+\.\d+\.\d+$', version):
            self.errors.append(f"Invalid version format: '{version}' (expected X.Y.Z)")

        # Check for required child elements
        required_elements = ['requires', 'extension']
        for elem in required_elements:
            if root.find(elem) is None:
                self.warnings.append(f"Missing recommended element: {elem}")

        self.info.append(f"✅ {xml_file} structure is valid")

    def validate_kodi_api(self):
        """Check for deprecated Kodi API usage."""
        deprecated_apis = [
            (r'xbmc->Log\(', "xbmc->Log is deprecated. Use CLog::Log() instead."),
            (r'xbmc->Output\(', "xbmc->Output is deprecated. Use CLog::Log() instead."),
            (r'ADDON::CAddon\b', "ADDON::CAddon is deprecated. Use kodi::addon::CAddon instead."),
        ]

        search_paths = ['src', 'lib', 'visualization.matrix']
        for path in search_paths:
            if os.path.exists(path):
                for pattern, message in deprecated_apis:
                    self._search_files(path, pattern, message, self.warnings)

    def validate_visualization_requirements(self):
        """Check for required visualization methods."""
        required_methods = ['Create', 'Start', 'Stop', 'Render', 'AudioData']
        search_paths = ['src', 'visualization.matrix']

        for method in required_methods:
            found = False
            for path in search_paths:
                if os.path.exists(path):
                    if self._search_files(path, rf'\b{method}\b', None, None, check_existence=True):
                        found = True
                        break
            if not found:
                self.errors.append(f"Required visualization method not found: {method}")

        if all(self._search_files(path, r'gl\b|GL_\b|GLuint\b|GLfloat\b', None, None, check_existence=True) 
               for path in search_paths if os.path.exists(path)):
            self.info.append("✅ OpenGL usage detected")
        else:
            self.warnings.append("No OpenGL usage detected - is this a visualization addon?")

        if any(self._search_files(path, r'GLES\b|OPENGLES\b', None, None, check_existence=True) 
               for path in search_paths if os.path.exists(path)):
            self.info.append("✅ OpenGL ES compatibility detected")

    def validate_fft_usage(self):
        """Check FFT library usage."""
        fft_patterns = [r'kiss_fft\b', r'kiss_fftr\b']
        search_paths = ['src', 'lib']

        fft_found = any(
            self._search_files(path, pattern, None, None, check_existence=True)
            for path in search_paths if os.path.exists(path)
            for pattern in fft_patterns
        )

        if fft_found:
            self.info.append("✅ FFT library (kissfft) detected")
            
            # Check for FFT size validation
            if not any(self._search_files(path, r'power.*2\b|nfft.*2\b', None, None, check_existence=True)
                       for path in search_paths if os.path.exists(path)):
                self.warnings.append("Consider validating FFT size is a power of 2")
            
            # Check for input validation
            if not any(self._search_files(path, r'isfinite\b|isnan\b|isinf\b', None, None, check_existence=True)
                       for path in search_paths if os.path.exists(path)):
                self.warnings.append("Consider adding NaN/Inf validation for FFT input")
        else:
            self.warnings.append("No FFT library usage detected")

    def validate_thread_safety(self):
        """Check for thread safety issues."""
        # Check for manual memory management
        if any(self._search_files('src', r'\bnew \[\b|\bmalloc\b|\bfree\b|\bdelete\b', 
                                   "Manual memory management found. Consider using smart pointers.", 
                                   self.warnings, check_existence=True)):
            pass

        # Check for mutex usage
        if not any(self._search_files('src', r'std::mutex\b|std::lock_guard\b|std::unique_lock\b', 
                                       None, None, check_existence=True)):
            self.info.append("ℹ️ No mutex usage detected. Ensure thread safety if using multiple threads.")

    def _search_files(self, path, pattern, message, message_list=None, check_existence=False):
        """Search files for a regex pattern."""
        found = False
        for root, _, files in os.walk(path):
            for file in files:
                if file.endswith(('.cpp', '.h', '.c')):
                    filepath = os.path.join(root, file)
                    try:
                        with open(filepath, 'r', encoding='utf-8') as f:
                            content = f.read()
                            if re.search(pattern, content):
                                found = True
                                if not check_existence and message:
                                    if message_list is None:
                                        message_list = self.warnings
                                    message_list.append(f"{message} (found in {filepath})")
                    except (IOError, UnicodeDecodeError):
                        continue
        return found

    def run_all_checks(self):
        """Run all validation checks."""
        self.validate_addon_xml()
        self.validate_kodi_api()
        self.validate_visualization_requirements()
        self.validate_fft_usage()
        self.validate_thread_safety()

    def print_results(self):
        """Print validation results."""
        if self.errors:
            print("\n❌ Errors:")
            for error in self.errors:
                print(f"  - {error}")
        
        if self.warnings:
            print("\n⚠️  Warnings:")
            for warning in self.warnings:
                print(f"  - {warning}")
        
        if self.info:
            print("\nℹ️  Info:")
            for info in self.info:
                print(f"  - {info}")
        
        if not self.errors and not self.warnings:
            print("\n✅ All checks passed!")
        
        return len(self.errors) == 0


def main():
    validator = KodiAddonValidator()
    validator.run_all_checks()
    success = validator.print_results()
    sys.exit(0 if success else 1)


if __name__ == "__main__":
    main()
