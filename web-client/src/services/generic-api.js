import { DEFAULT_URL } from "./types";

class ApiCaller {
  constructor(baseUrl = '') {
    this.baseUrl = baseUrl;
  }

  async request(endpoint, method = 'GET', data = null, headers = {}) {
    const url = `${this.baseUrl}${endpoint}`;
    const options = {
      method,
      headers: {
        ...(method === 'OPTIONS' ? { 'Content-Type': 'application/json' } : {}),
        ...headers,
      },
    };

    if (data) {
      options.body = JSON.stringify(data);
    }

    try {
      const response = await fetch(url, options);

      if (!response.ok) {
        if (response.status === 405) {
          await this.options(endpoint);
          return await this.request(endpoint, method, data, headers);
        }

        throw new Error(`HTTP error! Status: ${response.status}`);
      }

      return await response.json();
    } catch (error) {
      console.error(`Error in API call: ${error.message}`);
      throw error;
    }
  }

  async get(endpoint, headers = {}) {
    return this.request(endpoint, 'GET', null, headers);
  }

  async post(endpoint, data, headers = {}) {
    return this.request(endpoint, 'POST', data, headers);
  }

  async options(endpoint) {
    return this.request(endpoint, 'OPTIONS', null, {});
  }
}

const api = new ApiCaller(DEFAULT_URL);

export default api;
