#!/usr/bin/env python3
"""
GymBike ESP32 Connection Test
Test connectivity and API responses from the ESP32 web server
"""

import sys
import requests
import json
from urllib.parse import urljoin

def test_connection(esp_ip):
    """Test connection to ESP32 web server"""
    
    base_url = f"http://{esp_ip}"
    
    print(f"\n{'='*60}")
    print(f"Testing ESP32 GymBike at {base_url}")
    print(f"{'='*60}\n")
    
    # Test 1: Basic connectivity
    print("1. Testing basic connectivity...")
    try:
        response = requests.get(base_url, timeout=5)
        print(f"   ✓ Connected! Status code: {response.status_code}")
        print(f"   Content-Type: {response.headers.get('Content-Type', 'N/A')}")
        print(f"   Content-Length: {len(response.content)} bytes")
        
        # Check if we got HTML
        if 'html' in response.headers.get('Content-Type', '').lower():
            print(f"   ✓ HTML content received")
            # Check for key elements
            if 'Gym Bike' in response.text:
                print(f"   ✓ Page title found")
            if 'cadence' in response.text.lower():
                print(f"   ✓ Cadence element found")
        else:
            print(f"   ✗ Expected HTML, got: {response.headers.get('Content-Type')}")
            
    except requests.exceptions.Timeout:
        print(f"   ✗ Connection timeout - check if ESP32 is powered on")
        return False
    except requests.exceptions.ConnectionError as e:
        print(f"   ✗ Connection failed: {e}")
        return False
    except Exception as e:
        print(f"   ✗ Error: {e}")
        return False
    
    print()
    
    # Test 2: API endpoint
    print("2. Testing /api/status endpoint...")
    try:
        api_url = urljoin(base_url, '/api/status')
        response = requests.get(api_url, timeout=5)
        print(f"   ✓ API responded! Status code: {response.status_code}")
        
        # Try to parse JSON
        try:
            data = response.json()
            print(f"   ✓ Valid JSON response")
            print(f"   Data keys: {list(data.keys())}")
            print(f"   Cadence: {data.get('cadence', 'N/A')} RPM")
            print(f"   Resistance: {data.get('resistance', 'N/A')}%")
            print(f"   Mode: {data.get('mode', 'N/A')}")
            print(f"   \n   Full response:")
            print(f"   {json.dumps(data, indent=4)}")
        except json.JSONDecodeError:
            print(f"   ✗ Invalid JSON response")
            print(f"   Response: {response.text[:200]}")
            
    except Exception as e:
        print(f"   ✗ API test failed: {e}")
        
    print()
    
    # Test 3: CORS headers
    print("3. Checking CORS headers...")
    try:
        response = requests.get(api_url, timeout=5)
        cors_origin = response.headers.get('Access-Control-Allow-Origin', None)
        if cors_origin:
            print(f"   ✓ CORS enabled: {cors_origin}")
        else:
            print(f"   ✗ No CORS headers found (may cause issues in browser)")
            
    except Exception as e:
        print(f"   ✗ CORS check failed: {e}")
        
    print()
    
    # Test 4: Favicon
    print("4. Testing /favicon.svg...")
    try:
        favicon_url = urljoin(base_url, '/favicon.svg')
        response = requests.get(favicon_url, timeout=5)
        if response.status_code == 200:
            print(f"   ✓ Favicon available")
        else:
            print(f"   ✗ Favicon status: {response.status_code}")
    except Exception as e:
        print(f"   ✗ Favicon test failed: {e}")
        
    print()
    print(f"{'='*60}")
    print("Test complete!")
    print(f"{'='*60}\n")
    
    return True

def main():
    if len(sys.argv) < 2:
        print("Usage: python test_esp32.py <ESP32_IP_ADDRESS>")
        print("Example: python test_esp32.py 192.168.1.100")
        sys.exit(1)
    
    esp_ip = sys.argv[1]
    test_connection(esp_ip)

if __name__ == "__main__":
    main()
