#include <bits/stdc++.h>
using namespace std;

namespace ply {
    struct Vertex {
        float x, y, z;
    };
    struct Face {
        vector<uint32_t> vertex_indices;
        vector<float> texcoord;
    };
    uint32_t vertex_count, face_count;
    vector<Vertex> vertices;
    vector<Face> faces;

    void check_format(istringstream &ls) {
        string token;
        ls >> token;
        if (token == "binary_little_endian") {
            ls >> token;
            assert (token == "1.0");
        }
    }

    void read_element(istringstream &ls) {
        string token;
        uint32_t val;
        ls >> token >> val;
        if (token == "vertex") {
            vertex_count = val;
        } else if (token == "face") {
            face_count = val;
        }
    }

    void skip_header(istream &is) {
        string line;
        while (getline(is, line)) {
            istringstream ls(line);
            string token;
            ls >> token;
            if (token == "format") check_format(ls);
            if (token == "element") read_element(ls);
            if (token == "end_header") break;
        }
    }

    float read_float32(istream &is) {
        float val;
        is.read(reinterpret_cast<char *>(&val), sizeof(float));
        return val;
    }

    uint8_t read_uint8(istream &is) {
        uint8_t val;
        is.read(reinterpret_cast<char *>(&val), sizeof(val));
        return val;
    }

    uint32_t read_uint32(istream &is) {
        uint32_t val;
        is.read(reinterpret_cast<char *>(&val), sizeof(val));
        return val;
    }
    bool debug = false;

    void read_ply_file(const string &filepath) {
        ifstream ss(filepath, ios::binary);
        if (ss.fail()) throw runtime_error("failed to open " + filepath);
        skip_header(ss);
        vertices = vector<Vertex>(vertex_count);
        for (int i = 0; i < vertex_count; ++i) {
            vertices[i].x = read_float32(ss);
            vertices[i].y = read_float32(ss);
            vertices[i].z = read_float32(ss);
            if (debug) cout << vertices[i].x << " " << vertices[i].y << " " << vertices[i].z << endl;
        }
        faces = vector<Face>(face_count);
        for (int i = 0; i < face_count; ++i) {
            faces[i].vertex_indices = vector<uint32_t>(read_uint8(ss));
            for (auto &t: faces[i].vertex_indices) {
                t = read_uint32(ss);
                if (debug) cout << t << " ";
            }
            faces[i].texcoord = vector<float>(read_uint8(ss));
            for (auto &t: faces[i].texcoord) {
                t = read_float32(ss);
                if (debug) cout << t << " ";
            }
            if (debug) cout << endl;
        }
    }
}